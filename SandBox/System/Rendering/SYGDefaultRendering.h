#pragma once
#include "../../../Backend/Common.h"
#include "../../../Backend/Scene/SNSystemBase.h"
#include "../../../Backend/Scene/SNImpl.h"
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
#include "../../SBComponentType.h"
#include "../../SBRendererType.h"

namespace SandBox {
    class SYGDefaultRendering: public Scene::SNSystemBase {
        private:
            struct GDefaultRenderingInfo {
                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Log::LGImpl* logObj;
                    Renderer::VKSwapChain* swapChainObj;
                    Renderer::VKBuffer* vertexBufferObj;
                    Renderer::VKBuffer* indexBufferObj;
                    std::vector <Renderer::VKBuffer*> meshInstanceBufferObjs;
                    Renderer::VKRenderPass* renderPassObj;
                    Renderer::VKFrameBuffer* frameBufferObj;
                    Renderer::VKPipeline* pipelineObj;
                    Renderer::VKDescriptorSet* perFrameDescSetObj;
                    Renderer::VKDescriptorSet* otherDescSetObj;
                    Renderer::VKCmdBuffer* cmdBufferObj;
                    Renderer::VKRenderer* rendererObj;
                } resource;
            } m_gDefaultRenderingInfo;

        public:
            SYGDefaultRendering (void) {
                m_gDefaultRenderingInfo = {};

                auto& logObj = m_gDefaultRenderingInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",    __FILE__);
                logObj->updateLogConfig (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
            }

            void initGDefaultRenderingInfo (Scene::SNImpl* sceneObj,
                                            Collection::CNImpl* collectionObj) {

                if (sceneObj == nullptr || collectionObj == nullptr) {
                    LOG_ERROR (m_gDefaultRenderingInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                        << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }

                auto& resource              = m_gDefaultRenderingInfo.resource;
                resource.sceneObj           = sceneObj;
                resource.swapChainObj       = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain>     (
                    "CORE"
                );
                resource.vertexBufferObj    = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer>        (
                    "S:G_?_VERTEX"
                );
                resource.indexBufferObj     = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer>        (
                    "S:G_?_INDEX"
                );
                for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                    auto bufferObj          = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer>        (
                        "G_DEFAULT_MESH_INSTANCE_" + std::to_string (i)
                    );
                    resource.meshInstanceBufferObjs.push_back (bufferObj);
                }
                resource.renderPassObj      = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass>    (
                    "G"
                );
                resource.frameBufferObj     = collectionObj->getCollectionTypeInstance <Renderer::VKFrameBuffer>   (
                    "G"
                );
                resource.pipelineObj        = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>      (
                    "G_DEFAULT"
                );
                resource.perFrameDescSetObj = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorSet> (
                    "G_DEFAULT_PER_FRAME"
                );
                resource.otherDescSetObj    = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorSet> (
                    "G_DEFAULT_OTHER"
                );
                resource.cmdBufferObj       = collectionObj->getCollectionTypeInstance <Renderer::VKCmdBuffer>     (
                    "DRAW_OPS"
                );
                resource.rendererObj        = collectionObj->getCollectionTypeInstance <Renderer::VKRenderer>      (
                    "DRAW_OPS"
                );
            }

            void update (const void* meshInstances,
                         const void* activeCamera) {

                auto& resource            = m_gDefaultRenderingInfo.resource;
                auto& sceneObj            = resource.sceneObj;
                uint32_t frameInFlightIdx = resource.rendererObj->getFrameInFlightIdx();
                auto cmdBuffer            = resource.cmdBufferObj->getCmdBuffers()[frameInFlightIdx];
                /* Define the clear values to use for VK_ATTACHMENT_LOAD_OP_CLEAR. Note that, the order of clear values
                 * should be identical to the order of your attachments
                 *
                 * The range of depths in the depth buffer is 0.0 to 1.0 in Vulkan, where 1.0 lies at the far view plane
                 * and 0.0 at the near view plane. The initial value at each point in the depth buffer should be the
                 * furthest possible depth, which is 1.0
                */
                auto clearValues          = std::vector {
                    VkClearValue {                              /* Attachment idx 0 */
                        {{0.0f, 0.0f, 0.0f, 1.0f}}
                    },
                    VkClearValue {                              /* Attachment idx 1 */
                        {{0.0f, 0.0f, 0.0f, 1.0f}}
                    },
                    VkClearValue {                              /* Attachment idx 2 */
                        {{0.0f, 0.0f, 0.0f, 1.0f}}
                    },
                    VkClearValue {                              /* Attachment idx 3 */
                        {{0.0f, 0.0f, 0.0f, 1.0f}}
                    },
                    VkClearValue {                              /* Attachment idx 4 */
                        {{0.0f, 0.0f, 0.0f, 1.0f}}
                    },
                    VkClearValue {                              /* Attachment idx 5 */
                        {{1.0f, 0}}
                    }
                };

                /* Update buffer */
                resource.meshInstanceBufferObjs[frameInFlightIdx]->updateBuffer (
                    meshInstances,
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
                /* View ports */
                auto viewPorts = std::vector <VkViewport> {};
                Renderer::setViewPorts (
                    cmdBuffer,
                    0.0f,
                    0.0f,
                    (*resource.swapChainObj->getSwapChainExtent()).width,
                    (*resource.swapChainObj->getSwapChainExtent()).height,
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
                /* Vertex buffers */
                auto vertexBuffers       = std::vector <VkBuffer>     {
                    *resource.vertexBufferObj->getBuffer()
                };
                auto vertexBufferOffsets = std::vector <VkDeviceSize> {
                    0
                };
                Renderer::bindVertexBuffers (
                    cmdBuffer,
                    0,
                    vertexBuffers,
                    vertexBufferOffsets
                );
                /* Index buffer */
                Renderer::bindIndexBuffer (
                    cmdBuffer,
                    *resource.indexBufferObj->getBuffer(),
                    0,
                    VK_INDEX_TYPE_UINT32
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
                for (auto const& entity: m_entities) {
                    auto renderComponent = sceneObj->getComponent <RenderComponent> (entity);

                    Renderer::drawIndexed (
                        cmdBuffer,
                        renderComponent->m_firstIndexIdx,
                        renderComponent->m_indicesCount,
                        renderComponent->m_vertexOffset,
                        renderComponent->m_firstInstanceIdx,
                        renderComponent->m_instancesCount
                    );
                }
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

            ~SYGDefaultRendering (void) {
                delete m_gDefaultRenderingInfo.resource.logObj;
            }
    };
}   // namespace SandBox