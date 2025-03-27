#pragma once
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include "../../Backend/Scene/SNSystemBase.h"
#include "../../Backend/Scene/SNImpl.h"
#include "../../Backend/Log/LGImpl.h"
#include "../../Backend/Scene/SNType.h"
#include "../SBComponentType.h"
#include "../SBRendererType.h"

namespace SandBox {
    class SYMeshBatching: public Scene::SNSystemBase {
        private:
            struct OffsetInfo {
                uint32_t firstIndexIdx;
                uint32_t indicesCount;
                int32_t  vertexOffset;
            };

            struct MeshBatchingInfo {
                struct Meta {
                    /* Used for report purpose only */
                    std::unordered_map <Scene::Entity, OffsetInfo> entityToOffsetInfoMap;
                    std::vector <Vertex> vertices;
                    std::vector <IndexType> indices;
                } meta;

                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Log::LGImpl* logObj;
                } resource;
            } m_meshBatchingInfo;

        public:
            SYMeshBatching (void) {
                m_meshBatchingInfo = {};

                auto& logObj = m_meshBatchingInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",    __FILE__);
                logObj->updateLogConfig (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
            }

            void initMeshBatchingInfo (Scene::SNImpl* sceneObj) {
                m_meshBatchingInfo.meta.entityToOffsetInfoMap = {};
                m_meshBatchingInfo.meta.vertices              = {};
                m_meshBatchingInfo.meta.indices               = {};

                if (sceneObj == nullptr) {
                    LOG_ERROR (m_meshBatchingInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                   << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_meshBatchingInfo.resource.sceneObj = sceneObj;
            }

            std::vector <Vertex>& getBatchedVertices (void) {
                return m_meshBatchingInfo.meta.vertices;
            }

            std::vector <IndexType>& getBatchedIndices (void) {
                return m_meshBatchingInfo.meta.indices;
            }

            void update (void) {
                auto& sceneObj = m_meshBatchingInfo.resource.sceneObj;
                /* Clear previous batched data */
                m_meshBatchingInfo.meta.entityToOffsetInfoMap.clear();
                m_meshBatchingInfo.meta.vertices.clear();
                m_meshBatchingInfo.meta.indices.clear();

                size_t verticesCount     = 0;
                size_t indicesCount      = 0;
                uint32_t instancesCount  = 0;
                for (auto const& entity: m_entities) {
                    auto meshComponent   = sceneObj->getComponent <MeshComponent>   (entity);
                    auto renderComponent = sceneObj->getComponent <RenderComponent> (entity);

                    /* Populate render component */
                    renderComponent->m_firstIndexIdx    = static_cast <uint32_t> (indicesCount);
                    renderComponent->m_indicesCount     = static_cast <uint32_t> (meshComponent->m_indices.size());
                    renderComponent->m_vertexOffset     = static_cast <int32_t>  (verticesCount);
                    renderComponent->m_firstInstanceIdx = instancesCount;

                    verticesCount  += meshComponent->m_vertices.size();
                    indicesCount   += meshComponent->m_indices.size();
                    instancesCount += renderComponent->m_instancesCount;

                    m_meshBatchingInfo.meta.entityToOffsetInfoMap[entity] = {
                        renderComponent->m_firstIndexIdx,
                        renderComponent->m_indicesCount,
                        renderComponent->m_vertexOffset
                    };
                    m_meshBatchingInfo.meta.vertices.reserve (verticesCount);
                    m_meshBatchingInfo.meta.indices.reserve  (indicesCount);

                    m_meshBatchingInfo.meta.vertices.insert (
                        m_meshBatchingInfo.meta.vertices.end(),
                        meshComponent->m_vertices.begin(),
                        meshComponent->m_vertices.end()
                    );
                    m_meshBatchingInfo.meta.indices.insert (
                        m_meshBatchingInfo.meta.indices.end(),
                        meshComponent->m_indices.begin(),
                        meshComponent->m_indices.end()
                    );
                }
            }

            void generateReport (void) {
                auto& logObj = m_meshBatchingInfo.resource.logObj;

                for (auto const& [entity, info]: m_meshBatchingInfo.meta.entityToOffsetInfoMap) {
                    uint32_t firstIndexIdx = info.firstIndexIdx;
                    uint32_t lastIndexIdx  = firstIndexIdx + info.indicesCount;
                    size_t loopIdx         = 0;

                    LOG_LITE_INFO (logObj) << entity << std::endl;
                    LOG_LITE_INFO (logObj) << "{"    << std::endl;
                    for (uint32_t indexIdx = firstIndexIdx; indexIdx < lastIndexIdx; indexIdx++) {
                        auto index  = m_meshBatchingInfo.meta.indices[indexIdx];
                        auto vertex = m_meshBatchingInfo.meta.vertices[info.vertexOffset + index];

                        /* New line after every 3rd vertex */
                        if (loopIdx > 0 && (loopIdx % 3 == 0))
                        LOG_LITE_INFO (logObj) << std::endl;

                        LOG_LITE_INFO (logObj) << "\t" << "("
                                                       << ALIGN_AND_PAD_S << vertex.meta.uv.x       << ", "
                                                       << ALIGN_AND_PAD_S << vertex.meta.uv.y
                                                       << ")"
                                                       << " "
                                                       << "("
                                                       << ALIGN_AND_PAD_S << vertex.meta.normal.x   << ", "
                                                       << ALIGN_AND_PAD_S << vertex.meta.normal.y   << ", "
                                                       << ALIGN_AND_PAD_S << vertex.meta.normal.z
                                                       << ")"
                                                       << " "
                                                       << "("
                                                       << ALIGN_AND_PAD_S << vertex.meta.position.x << ", "
                                                       << ALIGN_AND_PAD_S << vertex.meta.position.y << ", "
                                                       << ALIGN_AND_PAD_S << vertex.meta.position.z
                                                       << ")"
                                                       << " "
                                                       << ALIGN_AND_PAD_S << +vertex.material.diffuseTextureIdx  << ", "
                                                       << ALIGN_AND_PAD_S << +vertex.material.specularTextureIdx << ", "
                                                       << ALIGN_AND_PAD_S << +vertex.material.emissionTextureIdx << ", "
                                                       << ALIGN_AND_PAD_S << vertex.material.shininess
                                                       << std::endl;
                        ++loopIdx;
                    }
                    LOG_LITE_INFO (logObj) << "}" << std::endl;
                }
            }

            ~SYMeshBatching (void) {
                delete m_meshBatchingInfo.resource.logObj;
            }
    };
}   // namespace SandBox