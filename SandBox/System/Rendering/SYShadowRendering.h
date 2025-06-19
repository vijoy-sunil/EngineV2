#pragma once
#include "../../../Backend/Common.h"
#include "../../../Backend/Scene/SNSystemBase.h"
#include "../../../Backend/Scene/SNImpl.h"
#include "../../../Backend/Collection/CNImpl.h"
#include "../../../Backend/Log/LGImpl.h"
#include "../../../Backend/Renderer/VKBuffer.h"
#include "../../../Backend/Renderer/VKImage.h"
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
    class SYShadowRendering: public Scene::SNSystemBase {
        private:
            struct ShadowRenderingInfo {
                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Log::LGImpl* logObj;
                    Renderer::VKBuffer* vertexBufferObj;
                    Renderer::VKBuffer* indexBufferObj;
                    std::vector <Renderer::VKBuffer*> meshInstanceBufferObjs;
                    Renderer::VKImage* depthImageObj;
                    Renderer::VKRenderPass* renderPassObj;
                    std::vector <Renderer::VKFrameBuffer*> frameBufferObjs;
                    Renderer::VKPipeline* pipelineObj;
                    Renderer::VKDescriptorSet* perFrameDescSetObj;
                    Renderer::VKCmdBuffer* cmdBufferObj;
                    Renderer::VKRenderer* rendererObj;
                } resource;
            } m_shadowRenderingInfo;

        public:
            SYShadowRendering (void) {
                m_shadowRenderingInfo = {};

                auto& logObj = m_shadowRenderingInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",    __FILE__);
                logObj->updateLogConfig (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
            }

            void initShadowRenderingInfo (const uint32_t activeLightsCount,
                                          Scene::SNImpl* sceneObj,
                                          Collection::CNImpl* collectionObj) {

                if (sceneObj == nullptr || collectionObj == nullptr) {
                    LOG_ERROR (m_shadowRenderingInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                      << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }

                auto& resource              = m_shadowRenderingInfo.resource;
                resource.sceneObj           = sceneObj;
                resource.vertexBufferObj    = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer>        (
                    "S:G_?_VERTEX"
                );
                resource.indexBufferObj     = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer>        (
                    "S:G_?_INDEX"
                );
                for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                    auto bufferObj          = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer>        (
                        "S_DEFAULT_MESH_INSTANCE_" + std::to_string (i)
                    );
                    resource.meshInstanceBufferObjs.push_back (bufferObj);
                }
                resource.depthImageObj      = collectionObj->getCollectionTypeInstance <Renderer::VKImage>         (
                    "S:F_?_DEPTH_0"         /* Use the first depth image */
                );
                resource.renderPassObj      = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass>    (
                    "S"
                );
                /*  Frame buffers
                 *  +-------+   +-------+-------+   +-------+-------+   +-------+-------+   +-------+
                 *  |  S_0  |...|  S_?  |  S_?  |...|  S_?  |  S_?  |...|  S_?  |  S_?  |...|  S_?  |
                 *  +-------+   +-------+-------+   +-------+-------+   +-------+-------+   +-------+
                 *  ^                   ^                   ^         x6                  x6        ^
                 *  |                   |                   |                                       |
                 *  +-------------------+-------------------+---------------------------------------+
                 *  Sun lights    +     Spot lights    +    Point lights (x6)   =                   Active lights count
                 *
                 * Note that, we are storing frame buffers for every active light (not just the sun and spot lights) for
                 * ease of indexing
                */
                for (uint32_t i = 0; i < activeLightsCount; i++) {
                    auto bufferObj          = collectionObj->getCollectionTypeInstance <Renderer::VKFrameBuffer>   (
                        "S_"                       + std::to_string (i)
                    );
                    resource.frameBufferObjs.push_back (bufferObj);
                }
                resource.pipelineObj        = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>      (
                    "S_DEFAULT"
                );
                resource.perFrameDescSetObj = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorSet> (
                    "S_DEFAULT_PER_FRAME"
                );
                resource.cmdBufferObj       = collectionObj->getCollectionTypeInstance <Renderer::VKCmdBuffer>     (
                    "DRAW_OPS"
                );
                resource.rendererObj        = collectionObj->getCollectionTypeInstance <Renderer::VKRenderer>      (
                    "DRAW_OPS"
                );
            }

            void update (const uint32_t activeLightIdx,
                         const void* meshInstances,
                         const void* activeLight) {

                auto& resource            = m_shadowRenderingInfo.resource;
                auto& sceneObj            = resource.sceneObj;
                uint32_t frameInFlightIdx = resource.rendererObj->getFrameInFlightIdx();
                auto frameBufferObj       = resource.frameBufferObjs[activeLightIdx];
                auto cmdBuffer            = resource.cmdBufferObj->getCmdBuffers()[frameInFlightIdx];
                auto clearValues          = std::vector {
                    VkClearValue {                              /* Attachment idx 0 */
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
                    *frameBufferObj->getFrameBuffer(),
                    {0, 0},
                    resource.depthImageObj->getImageExtent(),
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
                    sizeof (ActiveLightPC),
                    activeLight
                );
                /* View ports */
                auto viewPorts = std::vector <VkViewport> {};
                Renderer::setViewPorts (
                    cmdBuffer,
                    0.0f,
                    0.0f,
                    resource.depthImageObj->getImageExtent().width,
                    resource.depthImageObj->getImageExtent().height,
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
                    resource.depthImageObj->getImageExtent(),
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

            ~SYShadowRendering (void) {
                delete m_shadowRenderingInfo.resource.logObj;
            }
    };
}   // namespace SandBox