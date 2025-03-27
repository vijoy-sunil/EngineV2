#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <string>
#include <vector>
#include "../../Backend/Scene/SNSystemBase.h"
#include "../../Backend/Scene/SNImpl.h"
#include "../../Backend/Collection/CNImpl.h"
#include "../../Backend/Log/LGImpl.h"
#include "../../Backend/Renderer/VKSwapChain.h"
#include "../../Backend/Renderer/VKBuffer.h"
#include "../../Backend/Renderer/VKRenderPass.h"
#include "../../Backend/Renderer/VKFrameBuffer.h"
#include "../../Backend/Renderer/VKPipeline.h"
#include "../../Backend/Renderer/VKDescriptorSet.h"
#include "../../Backend/Renderer/VKCmdBuffer.h"
#include "../../Backend/Renderer/VKRenderer.h"
#include "../../Backend/Renderer/VKCmdList.h"
#include "../SBComponentType.h"
#include "../SBRendererType.h"

namespace SandBox {
    class SYDefaultRendering: public Scene::SNSystemBase {
        private:
            struct DefaultRenderingInfo {
                struct Meta {
                    Renderer::VKSwapChain*                 swapChainObj;
                    Renderer::VKBuffer*                    vertexBufferObj;
                    Renderer::VKBuffer*                    indexBufferObj;
                    std::vector <Renderer::VKBuffer*>      meshInstanceBufferObjs;
                    std::vector <Renderer::VKBuffer*>      lightInstanceBufferObjs;
                    Renderer::VKRenderPass*                renderPassObj;
                    std::vector <Renderer::VKFrameBuffer*> frameBufferObjs;
                    Renderer::VKPipeline*                  pipelineObj;
                    Renderer::VKDescriptorSet*             perFrameDescSetObj;
                    Renderer::VKDescriptorSet*             oneTimeDescSetObj;
                    Renderer::VKCmdBuffer*                 cmdBufferObj;
                    Renderer::VKRenderer*                  rendererObj;
                } meta;

                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Log::LGImpl* logObj;
                } resource;
            } m_defaultRenderingInfo;

        public:
            SYDefaultRendering (void) {
                m_defaultRenderingInfo = {};

                auto& logObj = m_defaultRenderingInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",    __FILE__);
                logObj->updateLogConfig (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
            }

            void initDefaultRenderingInfo (Scene::SNImpl* sceneObj,
                                           Collection::CNImpl* collectionObj) {

                if (sceneObj == nullptr || collectionObj == nullptr) {
                    LOG_ERROR (m_defaultRenderingInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                       << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }

                auto& meta              = m_defaultRenderingInfo.meta;
                meta.swapChainObj       = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain>     (
                    "DEFAULT"
                );
                meta.vertexBufferObj    = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer>        (
                    "DEFAULT_VERTEX"
                );
                meta.indexBufferObj     = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer>        (
                    "DEFAULT_INDEX"
                );
                for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                    auto bufferObj      = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer>        (
                        "DEFAULT_MESH_INSTANCE_" + std::to_string (i)
                    );
                    meta.meshInstanceBufferObjs.push_back (bufferObj);
                }
                for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                    auto bufferObj      = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer>        (
                        "DEFAULT_LIGHT_INSTANCE_" + std::to_string (i)
                    );
                    meta.lightInstanceBufferObjs.push_back (bufferObj);
                }
                meta.renderPassObj      = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass>    (
                    "DEFAULT"
                );
                for (uint32_t i = 0; i < meta.swapChainObj->getSwapChainImagesCount(); i++) {
                    auto bufferObj      = collectionObj->getCollectionTypeInstance <Renderer::VKFrameBuffer>   (
                        "DEFAULT_" + std::to_string (i)
                    );
                    meta.frameBufferObjs.push_back (bufferObj);
                }
                meta.pipelineObj        = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>      (
                    "DEFAULT"
                );
                meta.perFrameDescSetObj = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorSet> (
                    "DEFAULT_PER_FRAME"
                );
                meta.oneTimeDescSetObj  = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorSet> (
                    "DEFAULT_ONE_TIME"
                );
                meta.cmdBufferObj       = collectionObj->getCollectionTypeInstance <Renderer::VKCmdBuffer>     (
                    "DEFAULT_DRAW_OPS"
                );
                meta.rendererObj        = collectionObj->getCollectionTypeInstance <Renderer::VKRenderer>      (
                    "DEFAULT"
                );
                m_defaultRenderingInfo.resource.sceneObj = sceneObj;
            }

            void update (const void* batchedMeshInstances,
                         const void* batchedLightInstances,
                         const void* lightTypeOffsets,
                         const void* activeCamera) {

                auto& meta                 = m_defaultRenderingInfo.meta;
                auto& sceneObj             = m_defaultRenderingInfo.resource.sceneObj;

                uint32_t swapChainImageIdx = meta.rendererObj->getSwapChainImageIdx();
                uint32_t frameInFlightIdx  = meta.rendererObj->getFrameInFlightIdx();
                auto frameBufferObj        = meta.frameBufferObjs[swapChainImageIdx];
                auto cmdBuffer             = meta.cmdBufferObj->getCmdBuffers()[frameInFlightIdx];
                /* Define the clear values to use for VK_ATTACHMENT_LOAD_OP_CLEAR. Note that, the order of clear values
                 * should be identical to the order of your attachments
                 *
                 * The range of depths in the depth buffer is 0.0 to 1.0 in Vulkan, where 1.0 lies at the far view plane
                 * and 0.0 at the near view plane. The initial value at each point in the depth buffer should be the
                 * furthest possible depth, which is 1.0
                */
                auto clearValues           = std::vector {
                    VkClearValue {                          /* Attachment idx 0 */
                        {{0.0f, 0.0f, 0.0f, 1.0f}}
                    },
                    VkClearValue {                          /* Attachment idx 1 */
                        {{1.0f, 0}}
                    }
                };

                /* Update buffers */
                meta.meshInstanceBufferObjs[frameInFlightIdx]->updateBuffer (
                    batchedMeshInstances,
                    false
                );
                meta.lightInstanceBufferObjs[frameInFlightIdx]->updateBuffer (
                    batchedLightInstances,
                    false
                );
                /* Begin */
                Renderer::beginRenderPass (
                    cmdBuffer,
                    *meta.renderPassObj->getRenderPass(),
                    *frameBufferObj->getFrameBuffer(),
                    {0, 0},
                    *meta.swapChainObj->getSwapChainExtent(),
                    clearValues
                );
                /* Pipeline */
                Renderer::bindPipeline (
                    cmdBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    *meta.pipelineObj->getPipeline()
                );
                /* Push constants */
                Renderer::updatePushConstants (
                    cmdBuffer,
                    *meta.pipelineObj->getPipelineLayout(),
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    sizeof (ActiveCameraPC),
                    activeCamera
                );
                Renderer::updatePushConstants (
                    cmdBuffer,
                    *meta.pipelineObj->getPipelineLayout(),
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
                    (*meta.swapChainObj->getSwapChainExtent()).width,
                    (*meta.swapChainObj->getSwapChainExtent()).height,
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
                    *meta.swapChainObj->getSwapChainExtent(),
                    0,
                    scissors
                );
                /* Vertex buffers */
                auto vertexBuffers       = std::vector <VkBuffer>     {
                    *meta.vertexBufferObj->getBuffer()
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
                    *meta.indexBufferObj->getBuffer(),
                    0,
                    VK_INDEX_TYPE_UINT32
                );
                /* Descriptor sets */
                auto descriptorSets = std::vector {
                    meta.perFrameDescSetObj->getDescriptorSets()[frameInFlightIdx],
                    meta.oneTimeDescSetObj->getDescriptorSets()[0]
                };
                auto dynamicOffsets = std::vector <uint32_t> {};
                Renderer::bindDescriptorSets (
                    cmdBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    *meta.pipelineObj->getPipelineLayout(),
                    0,
                    descriptorSets,
                    dynamicOffsets
                );
                /* Draw */
                for (auto const& entity: m_entities) {
                    auto renderComponent = sceneObj->getComponent <RenderComponent> (entity);

                    if (renderComponent->m_tag == TAG_TYPE_DEFAULT) {
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
                /* End */
                Renderer::endRenderPass (
                    cmdBuffer
                );
            }

            ~SYDefaultRendering (void) {
                delete m_defaultRenderingInfo.resource.logObj;
            }
    };
}   // namespace SandBox