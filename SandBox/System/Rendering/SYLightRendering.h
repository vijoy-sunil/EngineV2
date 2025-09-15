#pragma once
#include "../../../Backend/Common.h"
#include "../../../Backend/Scene/SNSystemBase.h"
#include "../../../Backend/Collection/CNImpl.h"
#include "../../../Backend/Log/LGImpl.h"
#include "../../../Backend/Renderer/VKSwapChain.h"
#include "../../../Backend/Renderer/VKBuffer.h"
#include "../../../Backend/Renderer/VKRenderPass.h"
#include "../../../Backend/Renderer/VKFrameBuffer.h"
#include "../../../Backend/Renderer/VKPipeline.h"
#include "../../../Backend/Renderer/VKDescriptorSet.h"
#include "../../../Backend/Renderer/VKCmdBuffer.h"
#include "../../../Backend/Renderer/VKRenderer.h"
#include "../../../Backend/Renderer/VKCmdList.h"
#include "../../SBRendererType.h"

namespace SandBox {
    class SYLightRendering: public Scene::SNSystemBase {
        private:
            struct LightRenderingInfo {
                struct Resource {
                    Log::LGImpl* logObj;
                    Renderer::VKSwapChain* swapChainObj;
                    std::vector <Renderer::VKBuffer*> lightInstanceBufferObjs;
                    std::vector <Renderer::VKBuffer*> shadowConfigBufferObjs;
                    Renderer::VKRenderPass* renderPassObj;
                    Renderer::VKFrameBuffer* frameBufferObj;
                    Renderer::VKPipeline* pipelineObj;
                    Renderer::VKDescriptorSet* perFrameDescSetObj;
                    Renderer::VKDescriptorSet* otherDescSetObj;
                    Renderer::VKCmdBuffer* cmdBufferObj;
                    Renderer::VKRenderer* rendererObj;
                } resource;
            } m_lightRenderingInfo;

        public:
            SYLightRendering (void) {
                m_lightRenderingInfo = {};

                auto& logObj = m_lightRenderingInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",     __FILE__);
                logObj->updateLogConfig (Log::LEVEL_TYPE_INFO,    Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_WARNING, Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_ERROR,   Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
            }

            void initLightRenderingInfo (Collection::CNImpl* collectionObj) {
                auto& resource              = m_lightRenderingInfo.resource;
                if (collectionObj == nullptr) {
                    LOG_ERROR (resource.logObj) << NULL_DEPOBJ_MSG
                                                << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                resource.swapChainObj       = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain>     (
                    "CORE"
                );
                for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                    auto bufferObj          = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer>        (
                        "F_LIGHT_LIGHT_INSTANCE_" + std::to_string (i)
                    );
                    resource.lightInstanceBufferObjs.push_back (bufferObj);
                }
                for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                    auto bufferObj          = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer>        (
                        "F_LIGHT_SHADOW_CONFIG_"  + std::to_string (i)
                    );
                    resource.shadowConfigBufferObjs.push_back (bufferObj);
                }
                resource.renderPassObj      = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass>    (
                    "F"
                );
                resource.frameBufferObj     = collectionObj->getCollectionTypeInstance <Renderer::VKFrameBuffer>   (
                    "F"
                );
                resource.pipelineObj        = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>      (
                    "F_LIGHT"
                );
                resource.perFrameDescSetObj = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorSet> (
                    "F_LIGHT_PER_FRAME"
                );
                resource.otherDescSetObj    = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorSet> (
                    "F_LIGHT_OTHER"
                );
                resource.cmdBufferObj       = collectionObj->getCollectionTypeInstance <Renderer::VKCmdBuffer>     (
                    "DRAW_OPS"
                );
                resource.rendererObj        = collectionObj->getCollectionTypeInstance <Renderer::VKRenderer>      (
                    "DRAW_OPS"
                );
            }

            void update (const void* lightInstances,
                         const void* shadowConfig,
                         const void* lightTypeOffsets,
                         const void* activeCamera) {

                auto& resource            = m_lightRenderingInfo.resource;
                uint32_t frameInFlightIdx = resource.rendererObj->getFrameInFlightIdx();
                auto cmdBuffer            = resource.cmdBufferObj->getCmdBuffers()[frameInFlightIdx];
                auto clearValues          = std::vector {
                    VkClearValue {                              /* Attachment idx 0 */
                        {{0.0f, 0.0f, 0.0f, 1.0f}}
                    },
                    VkClearValue {                              /* Attachment idx 1 */
                        {{1.0f, 0}}
                    }
                };

                /* Update buffer */
                resource.lightInstanceBufferObjs[frameInFlightIdx]->updateBuffer (
                    lightInstances,
                    false
                );
                resource.shadowConfigBufferObjs[frameInFlightIdx]->updateBuffer  (
                    shadowConfig,
                    false
                );
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
                    *resource.frameBufferObj->getFrameBuffer(),
                    {0, 0},
                    *resource.swapChainObj->getSwapChainExtent(),
                    clearValues
                );
                /* Pipeline */
                Renderer::bindPipeline (
                    cmdBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    *resource.pipelineObj->getPipeline()
                );
                /* Push constants */
                Renderer::updatePushConstants (
                    cmdBuffer,
                    *resource.pipelineObj->getPipelineLayout(),
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    sizeof (ActiveCameraPC),
                    activeCamera
                );
                Renderer::updatePushConstants (
                    cmdBuffer,
                    *resource.pipelineObj->getPipelineLayout(),
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                    sizeof (ActiveCameraPC),
                    sizeof (LightTypeOffsetsPC),
                    lightTypeOffsets
                );
                /* View ports */
                auto viewPorts = std::vector <VkViewport> {};
                Renderer::setViewPorts (
                    cmdBuffer,
                    0.0f,
                    0.0f,
                    resource.swapChainObj->getSwapChainExtent()->width,
                    resource.swapChainObj->getSwapChainExtent()->height,
                    0.0f,
                    1.0f,
                    0,
                    viewPorts
                );
                /* Scissors */
                auto scissors = std::vector <VkRect2D> {};
                Renderer::setScissors (
                    cmdBuffer,
                    {0, 0},
                    *resource.swapChainObj->getSwapChainExtent(),
                    0,
                    scissors
                );
                /* Descriptor sets */
                auto descriptorSets = std::vector {
                    resource.perFrameDescSetObj->getDescriptorSets()[frameInFlightIdx],
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
                    6,
                    0,
                    1
                );
                /* [.] Continue render pass
                 *  .
                 *  .
                 *  .
                 *  .
                 * [.]
                */
            }

            ~SYLightRendering (void) {
                delete m_lightRenderingInfo.resource.logObj;
            }
    };
}   // namespace SandBox