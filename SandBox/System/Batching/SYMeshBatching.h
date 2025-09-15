#pragma once
#include "../../../Backend/Common.h"
#include "../../../Backend/Scene/SNSystemBase.h"
#include "../../../Backend/Scene/SNImpl.h"
#include "../../../Backend/Log/LGImpl.h"
#include "../../../Backend/Scene/SNType.h"
#include "../../SBComponentType.h"
#include "../../SBRendererType.h"

namespace SandBox {
    class SYMeshBatching: public Scene::SNSystemBase {
        private:
            struct OffsetInfo {
                uint32_t verticesCountPerPrimitive;
                uint32_t firstIndexIdx;
                uint32_t indicesCount;
                int32_t vertexOffset;
            };

            struct MeshBatchingInfo {
                struct Meta {
                    /* Used for report purpose only */
                    std::unordered_map <Scene::Entity, OffsetInfo> entityToOffsetInfoMap;
                    std::unordered_map <e_tagType, std::vector <Scene::Entity>> tagTypeToEntitiesMap;

                    std::unordered_map <e_tagType, std::vector <Vertex>> tagTypeToVerticesMap;
                    std::unordered_map <e_tagType, std::vector <IndexType>> tagTypeToIndicesMap;
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
                logObj->initLogInfo     ("Build/Log/SandBox",     __FILE__);
                logObj->updateLogConfig (Log::LEVEL_TYPE_INFO,    Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_WARNING, Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_ERROR,   Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
            }

            void initMeshBatchingInfo (Scene::SNImpl* sceneObj) {
                auto& meta                 = m_meshBatchingInfo.meta;
                auto& resource             = m_meshBatchingInfo.resource;

                meta.entityToOffsetInfoMap = {};
                meta.tagTypeToEntitiesMap  = {};
                meta.tagTypeToVerticesMap  = {};
                meta.tagTypeToIndicesMap   = {};

                if (sceneObj == nullptr) {
                    LOG_ERROR (resource.logObj) << NULL_DEPOBJ_MSG
                                                << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                resource.sceneObj          = sceneObj;
            }

            std::vector <Vertex>& getBatchedVertices (const e_tagType tagType) {
                return m_meshBatchingInfo.meta.tagTypeToVerticesMap[tagType];
            }

            std::vector <IndexType>& getBatchedIndices (const e_tagType tagType) {
                return m_meshBatchingInfo.meta.tagTypeToIndicesMap[tagType];
            }

            void update (void) {
                auto& meta     = m_meshBatchingInfo.meta;
                auto& sceneObj = m_meshBatchingInfo.resource.sceneObj;
                /* Clear previous batched data */
                meta.entityToOffsetInfoMap.clear();
                meta.tagTypeToEntitiesMap.clear();
                meta.tagTypeToVerticesMap.clear();
                meta.tagTypeToIndicesMap.clear();

                struct Counters {
                    size_t vertices;
                    size_t indices;
                    uint32_t instances;
                };
                std::unordered_map <e_tagType, Counters> tagTypeToCountersMap;
                for (auto const& entity: m_entities) {
                    auto metaComponent                  = sceneObj->getComponent <MetaComponent>   (entity);
                    auto meshComponent                  = sceneObj->getComponent <MeshComponent>   (entity);
                    auto renderComponent                = sceneObj->getComponent <RenderComponent> (entity);
                    auto& tagType                       = metaComponent->m_tagType;
                    auto& counters                      = tagTypeToCountersMap[tagType];
                    uint32_t verticesCountPerPrimitive  = tagType == TAG_TYPE_WIRE ? 2: 3;
                    /* Populate render component */
                    renderComponent->m_firstIndexIdx    = static_cast <uint32_t> (counters.indices);
                    renderComponent->m_indicesCount     = static_cast <uint32_t> (meshComponent->m_indices.size());
                    renderComponent->m_vertexOffset     = static_cast <int32_t>  (counters.vertices);
                    renderComponent->m_firstInstanceIdx = counters.instances;

                    counters.vertices                  += meshComponent->m_vertices.size();
                    counters.indices                   += meshComponent->m_indices.size();
                    counters.instances                 += renderComponent->m_instancesCount;

                    meta.entityToOffsetInfoMap[entity]  = {
                        verticesCountPerPrimitive,
                        renderComponent->m_firstIndexIdx,
                        renderComponent->m_indicesCount,
                        renderComponent->m_vertexOffset
                    };
                    meta.tagTypeToEntitiesMap[tagType].push_back (entity);

                    meta.tagTypeToVerticesMap[tagType].reserve (counters.vertices);
                    meta.tagTypeToVerticesMap[tagType].insert  (meta.tagTypeToVerticesMap[tagType].end(),
                                                                meshComponent->m_vertices.begin(),
                                                                meshComponent->m_vertices.end());

                    meta.tagTypeToIndicesMap[tagType].reserve  (counters.indices);
                    meta.tagTypeToIndicesMap[tagType].insert   (meta.tagTypeToIndicesMap[tagType].end(),
                                                                meshComponent->m_indices.begin(),
                                                                meshComponent->m_indices.end());
                }
            }

            void generateReport (void) {
                auto& meta     = m_meshBatchingInfo.meta;
                auto& resource = m_meshBatchingInfo.resource;
                auto& logObj   = resource.logObj;

                for (auto const& [tagType, entities]: meta.tagTypeToEntitiesMap) {
                    auto vertices = meta.tagTypeToVerticesMap[tagType];
                    auto indices  = meta.tagTypeToIndicesMap [tagType];

                    LOG_LITE_INFO (logObj)         << getTagTypeString (tagType)     << std::endl;
                    LOG_LITE_INFO (logObj)         << "["                            << std::endl;

                    for (auto const& entity: entities) {
                        auto metaComponent                 = resource.sceneObj->getComponent <MetaComponent> (entity);
                        auto info                          = meta.entityToOffsetInfoMap[entity];
                        uint32_t verticesCountPerPrimitive = info.verticesCountPerPrimitive;
                        uint32_t firstIndexIdx             = info.firstIndexIdx;
                        uint32_t lastIndexIdx              = firstIndexIdx + info.indicesCount;
                        size_t loopIdx                     = 0;

                        LOG_LITE_INFO (logObj)     << "\t"   << metaComponent->m_id  << std::endl;
                        LOG_LITE_INFO (logObj)     << "\t"   << "["                  << std::endl;

                        for (uint32_t indexIdx = firstIndexIdx; indexIdx < lastIndexIdx; indexIdx++) {
                            auto index  = indices [indexIdx];
                            auto vertex = vertices[info.vertexOffset + index];

                            /* New line after every primitive */
                            if (loopIdx > 0 && (loopIdx % verticesCountPerPrimitive == 0))
                            LOG_LITE_INFO (logObj) << std::endl;

                            LOG_LITE_INFO (logObj) << "\t\t" << "("
                                                             << ALIGN_AND_PAD_C (16) << vertex.meta.uv.x       << ", "
                                                             << ALIGN_AND_PAD_C (16) << vertex.meta.uv.y
                                                             << ")"
                                                             << ", "
                                                             << "("
                                                             << ALIGN_AND_PAD_C (16) << vertex.meta.normal.x   << ", "
                                                             << ALIGN_AND_PAD_C (16) << vertex.meta.normal.y   << ", "
                                                             << ALIGN_AND_PAD_C (16) << vertex.meta.normal.z
                                                             << ")"
                                                             << ", "
                                                             << "("
                                                             << ALIGN_AND_PAD_C (16) << vertex.meta.position.x << ", "
                                                             << ALIGN_AND_PAD_C (16) << vertex.meta.position.y << ", "
                                                             << ALIGN_AND_PAD_C (16) << vertex.meta.position.z
                                                             << ")"
                                                             << ", "
                                                             << ALIGN_AND_PAD_S      << vertex.material.diffuseTextureIdx
                                                             << ", "
                                                             << ALIGN_AND_PAD_S      << vertex.material.specularTextureIdx
                                                             << ", "
                                                             << ALIGN_AND_PAD_S      << vertex.material.emissionTextureIdx
                                                             << ", "
                                                             << ALIGN_AND_PAD_S      << vertex.material.shininess
                                                             << std::endl;
                            ++loopIdx;
                        }
                        LOG_LITE_INFO (logObj)     << "\t"   << "]"                  << std::endl;
                    }
                    LOG_LITE_INFO (logObj)         << "]"                            << std::endl;
                }
            }

            ~SYMeshBatching (void) {
                delete m_meshBatchingInfo.resource.logObj;
            }
    };
}   // namespace SandBox