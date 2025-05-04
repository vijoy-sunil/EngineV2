#pragma once
#include "../../Backend/Common.h"
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
                int32_t vertexOffset;
            };

            struct MeshBatchingInfo {
                struct Meta {
                    /* Used for report purpose only */
                    std::unordered_map <Scene::Entity, OffsetInfo> entityToOffsetInfoMap;
                    std::unordered_map <e_tagType, std::vector <Scene::Entity>> tagToEntitiesMap;

                    std::unordered_map <e_tagType, std::vector <Vertex>> tagToBatchedVerticesMap;
                    std::unordered_map <e_tagType, std::vector <IndexType>> tagToBatchedIndicesMap;
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
                m_meshBatchingInfo.meta.entityToOffsetInfoMap   = {};
                m_meshBatchingInfo.meta.tagToEntitiesMap        = {};
                m_meshBatchingInfo.meta.tagToBatchedVerticesMap = {};
                m_meshBatchingInfo.meta.tagToBatchedIndicesMap  = {};

                if (sceneObj == nullptr) {
                    LOG_ERROR (m_meshBatchingInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                   << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_meshBatchingInfo.resource.sceneObj = sceneObj;
            }

            std::vector <Vertex>& getBatchedVertices (const e_tagType tag) {
                return m_meshBatchingInfo.meta.tagToBatchedVerticesMap[tag];
            }

            std::vector <IndexType>& getBatchedIndices (const e_tagType tag) {
                return m_meshBatchingInfo.meta.tagToBatchedIndicesMap[tag];
            }

            void update (void) {
                auto& meta     = m_meshBatchingInfo.meta;
                auto& sceneObj = m_meshBatchingInfo.resource.sceneObj;
                /* Clear previous batched data */
                meta.entityToOffsetInfoMap.clear();
                meta.tagToEntitiesMap.clear();
                meta.tagToBatchedVerticesMap.clear();
                meta.tagToBatchedIndicesMap.clear();

                struct Counters {
                    size_t vertices;
                    size_t indices;
                    uint32_t instances;
                };
                std::unordered_map <e_tagType, Counters> tagToCountersMap;
                for (auto const& entity: m_entities) {
                    auto meshComponent                  = sceneObj->getComponent <MeshComponent>   (entity);
                    auto renderComponent                = sceneObj->getComponent <RenderComponent> (entity);
                    auto& counters                      = tagToCountersMap             [renderComponent->m_tag];
                    auto& batchedVertices               = meta.tagToBatchedVerticesMap [renderComponent->m_tag];
                    auto& batchedIndices                = meta.tagToBatchedIndicesMap  [renderComponent->m_tag];

                    /* Populate render component */
                    renderComponent->m_firstIndexIdx    = static_cast <uint32_t> (counters.indices);
                    renderComponent->m_indicesCount     = static_cast <uint32_t> (meshComponent->m_indices.size());
                    renderComponent->m_vertexOffset     = static_cast <int32_t>  (counters.vertices);
                    renderComponent->m_firstInstanceIdx = counters.instances;

                    counters.vertices                  += meshComponent->m_vertices.size();
                    counters.indices                   += meshComponent->m_indices.size();
                    counters.instances                 += renderComponent->m_instancesCount;

                    meta.entityToOffsetInfoMap[entity]  = {
                        renderComponent->m_firstIndexIdx,
                        renderComponent->m_indicesCount,
                        renderComponent->m_vertexOffset
                    };
                    meta.tagToEntitiesMap[renderComponent->m_tag].push_back (entity);

                    batchedVertices.reserve (counters.vertices);
                    batchedVertices.insert  (
                        batchedVertices.end(),
                        meshComponent->m_vertices.begin(),
                        meshComponent->m_vertices.end()
                    );

                    batchedIndices.reserve (counters.indices);
                    batchedIndices.insert  (
                        batchedIndices.end(),
                        meshComponent->m_indices.begin(),
                        meshComponent->m_indices.end()
                    );
                }
            }

            void generateReport (void) {
                auto& meta   = m_meshBatchingInfo.meta;
                auto& logObj = m_meshBatchingInfo.resource.logObj;

                for (auto const& [tag, entities]: meta.tagToEntitiesMap) {
                    auto batchedVertices = meta.tagToBatchedVerticesMap[tag];
                    auto batchedIndices  = meta.tagToBatchedIndicesMap[tag];

                    LOG_LITE_INFO (logObj) << tag << std::endl;
                    LOG_LITE_INFO (logObj) << "{" << std::endl;

                    for (auto const& entity: entities) {
                        auto info              = meta.entityToOffsetInfoMap[entity];
                        uint32_t firstIndexIdx = info.firstIndexIdx;
                        uint32_t lastIndexIdx  = firstIndexIdx + info.indicesCount;
                        size_t loopIdx         = 0;

                        LOG_LITE_INFO (logObj) << "\t" << entity << std::endl;
                        LOG_LITE_INFO (logObj) << "\t" << "{"    << std::endl;

                        for (uint32_t indexIdx = firstIndexIdx; indexIdx < lastIndexIdx; indexIdx++) {
                            auto index  = batchedIndices[indexIdx];
                            auto vertex = batchedVertices[info.vertexOffset + index];

                            /* New line after every 3rd vertex */
                            if (loopIdx > 0 && (loopIdx % 3 == 0))
                            LOG_LITE_INFO (logObj) << std::endl;

                            LOG_LITE_INFO (logObj) << "\t\t"
                                                   << "("
                                                   << ALIGN_AND_PAD_C (16) << vertex.meta.uv.x << ", "
                                                   << ALIGN_AND_PAD_C (16) << vertex.meta.uv.y
                                                   << ")"
                                                   << " "
                                                   << "("
                                                   << ALIGN_AND_PAD_C (16) << vertex.meta.normal.x << ", "
                                                   << ALIGN_AND_PAD_C (16) << vertex.meta.normal.y << ", "
                                                   << ALIGN_AND_PAD_C (16) << vertex.meta.normal.z
                                                   << ")"
                                                   << " "
                                                   << "("
                                                   << ALIGN_AND_PAD_C (16) << vertex.meta.position.x << ", "
                                                   << ALIGN_AND_PAD_C (16) << vertex.meta.position.y << ", "
                                                   << ALIGN_AND_PAD_C (16) << vertex.meta.position.z
                                                   << ")"
                                                   << " "
                                                   << ALIGN_AND_PAD_S << +vertex.material.diffuseTextureIdx  << ", "
                                                   << ALIGN_AND_PAD_S << +vertex.material.specularTextureIdx << ", "
                                                   << ALIGN_AND_PAD_S << +vertex.material.emissionTextureIdx << ", "
                                                   << ALIGN_AND_PAD_S << vertex.material.shininess
                                                   << std::endl;
                            ++loopIdx;
                        }
                        LOG_LITE_INFO (logObj) << "\t" << "}" << std::endl;
                    }
                    LOG_LITE_INFO (logObj) << "}" << std::endl;
                }
            }

            ~SYMeshBatching (void) {
                delete m_meshBatchingInfo.resource.logObj;
            }
    };
}   // namespace SandBox