#pragma once
#include "../../../Backend/Common.h"
#include "../../../Backend/Scene/SNSystemBase.h"
#include "../../../Backend/Scene/SNImpl.h"
#include "../../../Backend/Collection/CNImpl.h"
#include "../../../Backend/Log/LGImpl.h"
#include "../../../Backend/Renderer/VKBuffer.h"
#include "../../../Backend/Renderer/VKPipeline.h"
#include "../../../Backend/Renderer/VKDescriptorSet.h"
#include "../../../Backend/Renderer/VKCmdBuffer.h"
#include "../../../Backend/Renderer/VKRenderer.h"
#include "../../../Backend/Renderer/VKCmdList.h"
#include "../../SBComponentType.h"
#include "../../SBRendererType.h"

namespace SandBox {
    class SYFDefaultRendering: public Scene::SNSystemBase {
        private:
            struct FDefaultRenderingInfo {
                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Log::LGImpl* logObj;
                    Renderer::VKBuffer* vertexBufferObj;
                    Renderer::VKBuffer* indexBufferObj;
                    std::vector <Renderer::VKBuffer*> meshInstanceBufferObjs;
                    Renderer::VKPipeline* pipelineObj;
                    Renderer::VKDescriptorSet* perFrameDescSetObj;
                    Renderer::VKDescriptorSet* otherDescSetObj;
                    Renderer::VKCmdBuffer* cmdBufferObj;
                    Renderer::VKRenderer* rendererObj;
                } resource;
            } m_fDefaultRenderingInfo;

        public:
            SYFDefaultRendering (void) {
                m_fDefaultRenderingInfo = {};

                auto& logObj = m_fDefaultRenderingInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",    __FILE__);
                logObj->updateLogConfig (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
            }

            void initFDefaultRenderingInfo (Scene::SNImpl* sceneObj,
                                            Collection::CNImpl* collectionObj) {

                if (sceneObj == nullptr || collectionObj == nullptr) {
                    LOG_ERROR (m_fDefaultRenderingInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                        << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }

                auto& resource              = m_fDefaultRenderingInfo.resource;
                resource.sceneObj           = sceneObj;
                resource.vertexBufferObj    = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer>        (
                    "F_DEFAULT_VERTEX"
                );
                resource.indexBufferObj     = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer>        (
                    "F_DEFAULT_INDEX"
                );
                for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                    auto bufferObj          = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer>        (
                        "F_DEFAULT_MESH_INSTANCE_" + std::to_string (i)
                    );
                    resource.meshInstanceBufferObjs.push_back (bufferObj);
                }
                resource.pipelineObj        = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>      (
                    "F_DEFAULT"
                );
                resource.perFrameDescSetObj = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorSet> (
                    "F_DEFAULT_PER_FRAME"
                );
                resource.otherDescSetObj    = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorSet> (
                    "F_DEFAULT_OTHER"
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

                auto& resource            = m_fDefaultRenderingInfo.resource;
                auto& sceneObj            = resource.sceneObj;
                uint32_t frameInFlightIdx = resource.rendererObj->getFrameInFlightIdx();
                auto cmdBuffer            = resource.cmdBufferObj->getCmdBuffers()[frameInFlightIdx];

                /* Update buffer */
                resource.meshInstanceBufferObjs[frameInFlightIdx]->updateBuffer (
                    meshInstances,
                    false
                );
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
                /* Push constants */
                Renderer::updatePushConstants (
                    cmdBuffer,
                    *resource.pipelineObj->getPipelineLayout(),
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    sizeof (ActiveCameraPC),
                    activeCamera
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

                    if (renderComponent->m_tagType == TAG_TYPE_STD_ALPHA) {
                        Renderer::drawIndexed (
                            cmdBuffer,
                            renderComponent->m_firstIndexIdx,
                            renderComponent->m_indicesCount,
                            renderComponent->m_vertexOffset,
                            renderComponent->m_firstInstanceIdx,
                            renderComponent->m_instancesCount
                        );
                    }
                }
                /* [.] Continue render pass
                 *  .
                 *  .
                 *  .
                 *  .
                 * [.]
                */
            }

            ~SYFDefaultRendering (void) {
                delete m_fDefaultRenderingInfo.resource.logObj;
            }
    };
}   // namespace SandBox