#pragma once
#include "../../../Backend/Common.h"
#include "../../../Backend/Scene/SNSystemBase.h"
#include "../../../Backend/Scene/SNImpl.h"
#include "../../../Backend/Log/LGImpl.h"
#include "../../../Backend/Scene/SNType.h"
#include "../../SBComponentType.h"

namespace SandBox {
    class SYStdMeshInstanceBatching: public Scene::SNSystemBase {
        private:
            /* SBO - Storage buffer object */
            struct MeshInstanceLiteSBO {
                glm::mat4 modelMatrix;
            };

            struct MeshInstanceSBO {
                glm::mat4 modelMatrix;
                glm::mat4 normalMatrix;
                uint32_t textureIdxLUT[64];
            };

            struct StdMeshInstanceBatchingInfo {
                struct Meta {
                    /* Used for report purpose only */
                    std::unordered_map <Scene::Entity, size_t> entityToIdxMap;
                    std::unordered_map <e_tagType, std::vector <Scene::Entity>> tagTypeToEntitiesMap;

                    std::unordered_map <e_tagType, std::vector <MeshInstanceLiteSBO>> tagTypeToInstancesLiteMap;
                    std::unordered_map <e_tagType, std::vector <MeshInstanceSBO>> tagTypeToInstancesMap;
                } meta;

                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Log::LGImpl* logObj;
                } resource;
            } m_stdMeshInstanceBatchingInfo;

        public:
            SYStdMeshInstanceBatching (void) {
                m_stdMeshInstanceBatchingInfo = {};

                auto& logObj = m_stdMeshInstanceBatchingInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",     __FILE__);
                logObj->updateLogConfig (Log::LEVEL_TYPE_INFO,    Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_WARNING, Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_ERROR,   Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
            }

            void initStdMeshInstanceBatchingInfo (Scene::SNImpl* sceneObj) {
                m_stdMeshInstanceBatchingInfo.meta.entityToIdxMap            = {};
                m_stdMeshInstanceBatchingInfo.meta.tagTypeToEntitiesMap      = {};
                m_stdMeshInstanceBatchingInfo.meta.tagTypeToInstancesLiteMap = {};
                m_stdMeshInstanceBatchingInfo.meta.tagTypeToInstancesMap     = {};

                if (sceneObj == nullptr) {
                    LOG_ERROR (m_stdMeshInstanceBatchingInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                              << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_stdMeshInstanceBatchingInfo.resource.sceneObj = sceneObj;
            }

            std::vector <MeshInstanceLiteSBO>& getBatchedMeshInstancesLite (const e_tagType tagType) {
                return m_stdMeshInstanceBatchingInfo.meta.tagTypeToInstancesLiteMap[tagType];
            }

            std::vector <MeshInstanceSBO>& getBatchedMeshInstances (const e_tagType tagType) {
                return m_stdMeshInstanceBatchingInfo.meta.tagTypeToInstancesMap[tagType];
            }

            void update (void) {
                auto& meta     = m_stdMeshInstanceBatchingInfo.meta;
                auto& sceneObj = m_stdMeshInstanceBatchingInfo.resource.sceneObj;
                /* Clear previous batched data */
                meta.entityToIdxMap.clear();
                meta.tagTypeToEntitiesMap.clear();
                meta.tagTypeToInstancesLiteMap.clear();
                meta.tagTypeToInstancesMap.clear();

                std::unordered_map <e_tagType, size_t> tagTypeToLoopIdxMap;
                for (auto const& entity: m_entities) {
                    auto metaComponent          = sceneObj->getComponent <MetaComponent>          (entity);
                    auto transformComponent     = sceneObj->getComponent <TransformComponent>     (entity);
                    auto textureIdxLUTComponent = sceneObj->getComponent <TextureIdxLUTComponent> (entity);
                    auto& tagType               = metaComponent->m_tagType;
                    auto& loopIdx               = tagTypeToLoopIdxMap[tagType];
                    glm::mat4 modelMatrix       = transformComponent->createModelMatrix();

                    meta.entityToIdxMap[entity] = loopIdx++;
                    meta.tagTypeToEntitiesMap[tagType].push_back (entity);

                    MeshInstanceLiteSBO instanceLite;
                    instanceLite.modelMatrix    = modelMatrix;
                    meta.tagTypeToInstancesLiteMap[tagType].push_back (instanceLite);

                    MeshInstanceSBO instance;
                    instance.modelMatrix        = modelMatrix;
                    instance.normalMatrix       = glm::mat4 (glm::transpose (glm::inverse (glm::mat3 (modelMatrix))));
                    textureIdxLUTComponent->copyToTextureIdxLUT (instance.textureIdxLUT);
                    meta.tagTypeToInstancesMap[tagType].push_back (instance);
                }
            }

            void generateReport (void) {
                auto& meta     = m_stdMeshInstanceBatchingInfo.meta;
                auto& sceneObj = m_stdMeshInstanceBatchingInfo.resource.sceneObj;
                auto& logObj   = m_stdMeshInstanceBatchingInfo.resource.logObj;
                size_t rowIdx  = 0;

                for (auto const& [tagType, entities]: meta.tagTypeToEntitiesMap) {
                    auto instances = meta.tagTypeToInstancesMap[tagType];

                    LOG_LITE_INFO (logObj)     << getTagTypeString (tagType)       << std::endl;
                    LOG_LITE_INFO (logObj)     << "["                              << std::endl;

                    for (auto const& entity: entities) {
                        auto metaComponent          = sceneObj->getComponent <MetaComponent>          (entity);
                        auto textureIdxLUTComponent = sceneObj->getComponent <TextureIdxLUTComponent> (entity);
                        size_t idx                  = meta.entityToIdxMap[entity];
                        auto& modelMatrix           = instances[idx].modelMatrix;
                        auto& normalMatrix          = instances[idx].normalMatrix;
                        auto& textureIdxLUT         = instances[idx].textureIdxLUT;

                        LOG_LITE_INFO (logObj) << "\t"     << metaComponent->m_id  << std::endl;
                        LOG_LITE_INFO (logObj) << "\t"     << "{"                  << std::endl;
                        LOG_LITE_INFO (logObj) << "\t\t"   << "["                  << std::endl;
                        while (rowIdx < 4) {
                        LOG_LITE_INFO (logObj) << "\t\t\t" << ALIGN_AND_PAD_C (16) << modelMatrix[rowIdx][0]  << ", "
                                                           << ALIGN_AND_PAD_C (16) << modelMatrix[rowIdx][1]  << ", "
                                                           << ALIGN_AND_PAD_C (16) << modelMatrix[rowIdx][2]  << ", "
                                                           << ALIGN_AND_PAD_C (16) << modelMatrix[rowIdx][3]
                                                           << std::endl;
                        ++rowIdx;
                        }
                        rowIdx = 0;
                        LOG_LITE_INFO (logObj) << "\t\t"   << "]"                  << std::endl;
                        LOG_LITE_INFO (logObj) << "\t\t"   << "["                  << std::endl;
                        while (rowIdx < 4) {
                        LOG_LITE_INFO (logObj) << "\t\t\t" << ALIGN_AND_PAD_C (16) << normalMatrix[rowIdx][0] << ", "
                                                           << ALIGN_AND_PAD_C (16) << normalMatrix[rowIdx][1] << ", "
                                                           << ALIGN_AND_PAD_C (16) << normalMatrix[rowIdx][2] << ", "
                                                           << ALIGN_AND_PAD_C (16) << normalMatrix[rowIdx][3]
                                                           << std::endl;
                        ++rowIdx;
                        }
                        rowIdx = 0;
                        LOG_LITE_INFO (logObj) << "\t\t"   << "]"                  << std::endl;
                        LOG_LITE_INFO (logObj) << "\t\t"   << "["                  << std::endl;
                        for (size_t readIdx = 0; readIdx < 64; readIdx++) {
                        /* Split packet into texture indices */
                        uint32_t packet     = textureIdxLUT[readIdx];
                        auto newTextureIdx0 = textureIdxLUTComponent->decodeTextureIdx  (packet & 0x000000FF);
                        auto newTextureIdx1 = textureIdxLUTComponent->decodeTextureIdx ((packet & 0x0000FF00) >>  8);
                        auto newTextureIdx2 = textureIdxLUTComponent->decodeTextureIdx ((packet & 0x00FF0000) >> 16);
                        auto newTextureIdx3 = textureIdxLUTComponent->decodeTextureIdx ((packet & 0xFF000000) >> 24);

                        LOG_LITE_INFO (logObj) << "\t\t\t" << ALIGN_AND_PAD_C (3)  << readIdx                 << ": "
                                                           << ALIGN_AND_PAD_C (3)  << +newTextureIdx0         << ", "
                                                           << ALIGN_AND_PAD_C (3)  << +newTextureIdx1         << ", "
                                                           << ALIGN_AND_PAD_C (3)  << +newTextureIdx2         << ", "
                                                           << ALIGN_AND_PAD_C (3)  << +newTextureIdx3
                                                           << std::endl;
                        }
                        LOG_LITE_INFO (logObj) << "\t\t"   << "]"                  << std::endl;
                        LOG_LITE_INFO (logObj) << "\t"     << "}"                  << std::endl;
                    }
                    LOG_LITE_INFO (logObj)     << "]"                              << std::endl;
                }
            }

            ~SYStdMeshInstanceBatching (void) {
                delete m_stdMeshInstanceBatchingInfo.resource.logObj;
            }
    };
}   // namespace SandBox