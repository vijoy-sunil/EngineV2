#pragma once
#include "../../../Backend/Common.h"
#include "../../../Backend/Scene/SNSystemBase.h"
#include "../../../Backend/Collection/CNImpl.h"
#include "../../../Backend/Log/LGImpl.h"
#include "../../../Backend/Renderer/VKSwapChain.h"
#include "../../../Backend/Renderer/VKRenderPass.h"
#include "../../../Backend/Renderer/VKFrameBuffer.h"
#include "../../../Backend/Renderer/VKCmdBuffer.h"
#include "../../../Backend/Renderer/VKRenderer.h"
#include "../../../Backend/Renderer/VKCmdList.h"

namespace SandBox {
    class SYGuiRendering: public Scene::SNSystemBase {
        private:
            struct GuiRenderingInfo {
                struct Resource {
                    Log::LGImpl* logObj;
                    Renderer::VKSwapChain* swapChainObj;
                    Renderer::VKRenderPass* renderPassObj;
                    std::vector <Renderer::VKFrameBuffer*> frameBufferObjs;
                    Renderer::VKCmdBuffer* cmdBufferObj;
                    Renderer::VKRenderer* rendererObj;
                } resource;
            } m_guiRenderingInfo;

        public:
            SYGuiRendering (void) {
                m_guiRenderingInfo = {};

                auto& logObj = m_guiRenderingInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",     __FILE__);
                logObj->updateLogConfig (Log::LEVEL_TYPE_INFO,    Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_WARNING, Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_ERROR,   Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
            }

            void initGuiRenderingInfo (Collection::CNImpl* collectionObj) {
                if (collectionObj == nullptr) {
                    LOG_ERROR (m_guiRenderingInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                   << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }

                auto& resource         = m_guiRenderingInfo.resource;
                resource.swapChainObj  = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain>   (
                    "CORE"
                );
                resource.renderPassObj = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass>  (
                    "GUI"
                );
                for (uint32_t i = 0; i < resource.swapChainObj->getSwapChainImagesCount(); i++) {
                    auto bufferObj     = collectionObj->getCollectionTypeInstance <Renderer::VKFrameBuffer> (
                        "GUI_" + std::to_string (i)
                    );
                    resource.frameBufferObjs.push_back (bufferObj);
                }
                resource.cmdBufferObj  = collectionObj->getCollectionTypeInstance <Renderer::VKCmdBuffer>   (
                    "DRAW_OPS"
                );
                resource.rendererObj   = collectionObj->getCollectionTypeInstance <Renderer::VKRenderer>    (
                    "DRAW_OPS"
                );
            }

            void update (void) {
                auto& resource             = m_guiRenderingInfo.resource;
                uint32_t swapChainImageIdx = resource.rendererObj->getSwapChainImageIdx();
                uint32_t frameInFlightIdx  = resource.rendererObj->getFrameInFlightIdx();
                auto frameBufferObj        = resource.frameBufferObjs[swapChainImageIdx];
                auto cmdBuffer             = resource.cmdBufferObj->getCmdBuffers()[frameInFlightIdx];
                auto clearValues           = std::vector {
                    VkClearValue {                              /* Attachment idx 0 */
                        {{0.0f, 0.0f, 0.0f, 1.0f}}
                    }
                };

                /* [O] Begin render pass
                 *  .
                 *  .
                 *  .
                 *  .
                 * [.]
                */
                Renderer::beginRenderPass (
                    cmdBuffer,
                    *resource.renderPassObj->getRenderPass(),
                    *frameBufferObj->getFrameBuffer(),
                    {0, 0},
                    *resource.swapChainObj->getSwapChainExtent(),
                    clearValues
                );
                /* Draw */
                Renderer::drawGui (
                    cmdBuffer
                );
                /* [.]
                 *  .
                 *  .
                 *  .
                 *  .
                 * [O] End render pass
                */
                Renderer::endRenderPass (
                    cmdBuffer
                );
            }

            ~SYGuiRendering (void) {
                delete m_guiRenderingInfo.resource.logObj;
            }
    };
}   // namespace SandBox