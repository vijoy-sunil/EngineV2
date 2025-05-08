#pragma once
#include "../../Backend/Common.h"
#include "../../Backend/Scene/SNSystemBase.h"
#include "../../Backend/Scene/SNImpl.h"
#include "../../Backend/Log/LGImpl.h"
#include "../../Backend/Scene/SNType.h"
#include "../SBComponentType.h"

namespace SandBox {
    class SYWireMeshInstanceBatching: public Scene::SNSystemBase {
        private:
            /* SBO - Storage buffer object */
            struct MeshInstanceSBO {
                glm::mat4 modelMatrix;          /* Mat4 must be aligned by 4N (= 16 bytes) */
                glm::vec4 color;                /* Vec4 must be aligned by 4N (= 16 bytes) */
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
                logObj->initLogInfo     ("Build/Log/SandBox",    __FILE__);
                logObj->updateLogConfig (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
            }

            void initWireMeshInstanceBatchingInfo (Scene::SNImpl* sceneObj) {
                m_wireMeshInstanceBatchingInfo.meta.entityToIdxMap = {};
                m_wireMeshInstanceBatchingInfo.meta.instances      = {};

                if (sceneObj == nullptr) {
                    LOG_ERROR (m_wireMeshInstanceBatchingInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                               << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_wireMeshInstanceBatchingInfo.resource.sceneObj = sceneObj;
            }

            std::vector <MeshInstanceSBO>& getBatchedMeshInstances (void) {
                return m_wireMeshInstanceBatchingInfo.meta.instances;
            }

            void update (void) {
                auto& sceneObj = m_wireMeshInstanceBatchingInfo.resource.sceneObj;
                /* Clear previous batched data */
                m_wireMeshInstanceBatchingInfo.meta.entityToIdxMap.clear();
                m_wireMeshInstanceBatchingInfo.meta.instances.clear();

                size_t loopIdx = 0;
                for (auto const& entity: m_entities) {
                    auto transformComponent = sceneObj->getComponent <TransformComponent> (entity);
                    auto colorComponent     = sceneObj->getComponent <ColorComponent>     (entity);

                    m_wireMeshInstanceBatchingInfo.meta.entityToIdxMap[entity] = loopIdx++;
                    MeshInstanceSBO instance;
                    instance.modelMatrix = transformComponent->createModelMatrix();
                    instance.color       = colorComponent->m_color;
                    m_wireMeshInstanceBatchingInfo.meta.instances.push_back (instance);
                }
            }

            void generateReport (void) {
                auto& logObj  = m_wireMeshInstanceBatchingInfo.resource.logObj;
                size_t rowIdx = 0;

                for (auto const& [entity, idx]: m_wireMeshInstanceBatchingInfo.meta.entityToIdxMap) {
                    auto& modelMatrix = m_wireMeshInstanceBatchingInfo.meta.instances[idx].modelMatrix;
                    auto& color       = m_wireMeshInstanceBatchingInfo.meta.instances[idx].color;

                    LOG_LITE_INFO (logObj)     << entity << std::endl;
                    LOG_LITE_INFO (logObj)     << "{"    << std::endl;
                    LOG_LITE_INFO (logObj)     << "\t"   << "[" << std::endl;
                    while (rowIdx < 4) {
                        LOG_LITE_INFO (logObj) << "\t\t" << ALIGN_AND_PAD_C (16) << modelMatrix[rowIdx][0] << ", "
                                                         << ALIGN_AND_PAD_C (16) << modelMatrix[rowIdx][1] << ", "
                                                         << ALIGN_AND_PAD_C (16) << modelMatrix[rowIdx][2] << ", "
                                                         << ALIGN_AND_PAD_C (16) << modelMatrix[rowIdx][3]
                                                         << std::endl;
                        ++rowIdx;
                    }
                    LOG_LITE_INFO (logObj)     << "\t"   << "]" << std::endl;
                    rowIdx = 0;
                    LOG_LITE_INFO (logObj)     << "\t"   << "("
                                                         << ALIGN_AND_PAD_C (16) << color.r << ", "
                                                         << ALIGN_AND_PAD_C (16) << color.g << ", "
                                                         << ALIGN_AND_PAD_C (16) << color.b << ", "
                                                         << ALIGN_AND_PAD_C (16) << color.a
                                                         << ")"
                                                         << std::endl;
                    LOG_LITE_INFO (logObj)     << "}"    << std::endl;
                }
            }

            ~SYWireMeshInstanceBatching (void) {
                delete m_wireMeshInstanceBatchingInfo.resource.logObj;
            }
    };
}   // namespace SandBox