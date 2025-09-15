#pragma once
#include "../../../Backend/Common.h"
#include "../../../Backend/Scene/SNSystemBase.h"
#include "../../../Backend/Scene/SNImpl.h"
#include "../../../Backend/Log/LGImpl.h"
#include "../../../Backend/Scene/SNType.h"
#include "../../SBComponentType.h"

namespace SandBox {
    class SYWireMeshInstanceBatching: public Scene::SNSystemBase {
        private:
            /* SBO - Storage buffer object */
            struct MeshInstanceSBO {
                glm::vec3 color;
                alignas (16) glm::mat4 modelMatrix;
            };

            struct WireMeshInstanceBatchingInfo {
                struct Meta {
                    /* Used for report purpose only */
                    std::unordered_map <Scene::Entity, size_t> entityToIdxMap;
                    std::vector <MeshInstanceSBO> instances;
                } meta;

                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Log::LGImpl* logObj;
                } resource;
            } m_wireMeshInstanceBatchingInfo;

        public:
            SYWireMeshInstanceBatching (void) {
                m_wireMeshInstanceBatchingInfo = {};

                auto& logObj = m_wireMeshInstanceBatchingInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",     __FILE__);
                logObj->updateLogConfig (Log::LEVEL_TYPE_INFO,    Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_WARNING, Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_ERROR,   Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
            }

            void initWireMeshInstanceBatchingInfo (Scene::SNImpl* sceneObj) {
                auto& meta          = m_wireMeshInstanceBatchingInfo.meta;
                auto& resource      = m_wireMeshInstanceBatchingInfo.resource;

                meta.entityToIdxMap = {};
                meta.instances      = {};

                if (sceneObj == nullptr) {
                    LOG_ERROR (resource.logObj) << NULL_DEPOBJ_MSG
                                                << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                resource.sceneObj   = sceneObj;
            }

            std::vector <MeshInstanceSBO>& getBatchedMeshInstances (void) {
                return m_wireMeshInstanceBatchingInfo.meta.instances;
            }

            void update (void) {
                auto& meta     = m_wireMeshInstanceBatchingInfo.meta;
                auto& sceneObj = m_wireMeshInstanceBatchingInfo.resource.sceneObj;
                size_t loopIdx = 0;
                /* Clear previous batched data */
                meta.entityToIdxMap.clear();
                meta.instances.clear();

                for (auto const& entity: m_entities) {
                    auto transformComponent     = sceneObj->getComponent <TransformComponent> (entity);
                    auto colorComponent         = sceneObj->getComponent <ColorComponent>     (entity);

                    meta.entityToIdxMap[entity] = loopIdx++;

                    MeshInstanceSBO instance;
                    instance.color              = glm::vec3 (colorComponent->m_color);
                    instance.modelMatrix        = transformComponent->createModelMatrix();
                    meta.instances.push_back (instance);
                }
            }

            void generateReport (void) {
                auto& meta     = m_wireMeshInstanceBatchingInfo.meta;
                auto& resource = m_wireMeshInstanceBatchingInfo.resource;
                auto& logObj   = resource.logObj;
                size_t rowIdx  = 0;

                for (auto const& [entity, idx]: meta.entityToIdxMap) {
                    auto metaComponent = resource.sceneObj->getComponent <MetaComponent> (entity);
                    auto& color        = meta.instances[idx].color;
                    auto& modelMatrix  = meta.instances[idx].modelMatrix;

                    LOG_LITE_INFO (logObj) << metaComponent->m_id            << std::endl;
                    LOG_LITE_INFO (logObj) << "{"                            << std::endl;
                    LOG_LITE_INFO (logObj) << "\t"   << "("
                                                     << ALIGN_AND_PAD_C (16) << color.r                << ", "
                                                     << ALIGN_AND_PAD_C (16) << color.g                << ", "
                                                     << ALIGN_AND_PAD_C (16) << color.b
                                                     << ")"
                                                     << std::endl;
                    LOG_LITE_INFO (logObj) << "\t"   << "["                  << std::endl;
                    while (rowIdx < 4) {
                    LOG_LITE_INFO (logObj) << "\t\t" << ALIGN_AND_PAD_C (16) << modelMatrix[rowIdx][0] << ", "
                                                     << ALIGN_AND_PAD_C (16) << modelMatrix[rowIdx][1] << ", "
                                                     << ALIGN_AND_PAD_C (16) << modelMatrix[rowIdx][2] << ", "
                                                     << ALIGN_AND_PAD_C (16) << modelMatrix[rowIdx][3]
                                                     << std::endl;
                    ++rowIdx;
                    }
                    rowIdx = 0;
                    LOG_LITE_INFO (logObj) << "\t"   << "]"                  << std::endl;
                    LOG_LITE_INFO (logObj) << "}"                            << std::endl;
                }
            }

            ~SYWireMeshInstanceBatching (void) {
                delete m_wireMeshInstanceBatchingInfo.resource.logObj;
            }
    };
}   // namespace SandBox