#pragma once
#include "../../../Backend/Common.h"
#include "../../../Backend/Scene/SNSystemBase.h"
#include "../../../Backend/Collection/CNImpl.h"
#include "../../../Backend/Log/LGImpl.h"
#include "../../../Backend/Renderer/VKPipeline.h"
#include "../../../Backend/Renderer/VKDescriptorSet.h"
#include "../../../Backend/Renderer/VKCmdBuffer.h"
#include "../../../Backend/Renderer/VKRenderer.h"
#include "../../../Backend/Renderer/VKCmdList.h"

#define ENABLE_DEBUG_RENDERING      (false)

namespace SandBox {
    class SYDebugRendering: public Scene::SNSystemBase {
        private:
            struct DebugRenderingInfo {
                struct Resource {
                    Log::LGImpl* logObj;
                    Renderer::VKPipeline* pipelineObj;
                    Renderer::VKDescriptorSet* otherDescSetObj;
                    Renderer::VKCmdBuffer* cmdBufferObj;
                    Renderer::VKRenderer* rendererObj;
                } resource;
            } m_debugRenderingInfo;

        public:
            SYDebugRendering (void) {
                m_debugRenderingInfo = {};

                auto& logObj = m_debugRenderingInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",     __FILE__);
                logObj->updateLogConfig (Log::LEVEL_TYPE_INFO,    Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_WARNING, Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_ERROR,   Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
            }

            void initDebugRenderingInfo (Collection::CNImpl* collectionObj) {
                if (collectionObj == nullptr) {
                    LOG_ERROR (m_debugRenderingInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                     << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }

                auto& resource           = m_debugRenderingInfo.resource;
                resource.pipelineObj     = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>      (
                    "F_DEBUG"
                );
                resource.otherDescSetObj = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorSet> (
                    "F_DEBUG_OTHER"
                );
                resource.cmdBufferObj    = collectionObj->getCollectionTypeInstance <Renderer::VKCmdBuffer>     (
                    "DRAW_OPS"
                );
                resource.rendererObj     = collectionObj->getCollectionTypeInstance <Renderer::VKRenderer>      (
                    "DRAW_OPS"
                );
            }

            void update (void) {
                auto& resource            = m_debugRenderingInfo.resource;
                uint32_t frameInFlightIdx = resource.rendererObj->getFrameInFlightIdx();
                auto cmdBuffer            = resource.cmdBufferObj->getCmdBuffers()[frameInFlightIdx];

#if ENABLE_DEBUG_RENDERING
                /* [.] Continue render pass
                 *  .
                 *  .
                 *  .
                 *  .
                 * [.]
                */
                /* Pipeline */
                Renderer::bindPipeline (
                    cmdBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    *resource.pipelineObj->getPipeline()
                );
                /* Descriptor sets */
                auto descriptorSets = std::vector {
                    resource.otherDescSetObj->getDescriptorSets()[0]
                };
                auto dynamicOffsets = std::vector <uint32_t> {};
                Renderer::bindDescriptorSets (
                    cmdBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    *resource.pipelineObj->getPipelineLayout(),
                    0,
                    descriptorSets,
                    dynamicOffsets
                );
                /* Draw */
                Renderer::drawSimple (
                    cmdBuffer,
                    0,
                    48,
                    0,
                    1
                );
#endif  // ENABLE_DEBUG_RENDERING
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

            ~SYDebugRendering (void) {
                delete m_debugRenderingInfo.resource.logObj;
            }
    };
}   // namespace SandBox