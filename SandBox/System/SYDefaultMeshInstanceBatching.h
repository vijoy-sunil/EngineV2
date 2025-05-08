#pragma once
#include "../../Backend/Common.h"
#include "../../Backend/Scene/SNSystemBase.h"
#include "../../Backend/Scene/SNImpl.h"
#include "../../Backend/Log/LGImpl.h"
#include "../../Backend/Scene/SNType.h"
#include "../SBComponentType.h"

namespace SandBox {
    class SYDefaultMeshInstanceBatching: public Scene::SNSystemBase {
        private:
            /* SBO - Storage buffer object */
            struct MeshInstanceSBO {
                glm::mat4 modelMatrix;          /* Mat4 must be aligned by 4N (= 16 bytes) */
                glm::mat4 normalMatrix;
                uint32_t textureIdxLUT[64];     /* Scalars must be aligned by N (= 4 bytes given 32 bit floats) */
            };

            struct DefaultMeshInstanceBatchingInfo {
                struct Meta {
                    /* Used for report purpose only */
                    std::unordered_map <Scene::Entity, size_t> entityToIdxMap;
                    std::vector <MeshInstanceSBO> instances;
                } meta;

                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Log::LGImpl* logObj;
                } resource;
            } m_defaultMeshInstanceBatchingInfo;

        public:
            SYDefaultMeshInstanceBatching (void) {
                m_defaultMeshInstanceBatchingInfo = {};

                auto& logObj = m_defaultMeshInstanceBatchingInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",    __FILE__);
                logObj->updateLogConfig (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
            }

            void initDefaultMeshInstanceBatchingInfo (Scene::SNImpl* sceneObj) {
                m_defaultMeshInstanceBatchingInfo.meta.entityToIdxMap = {};
                m_defaultMeshInstanceBatchingInfo.meta.instances      = {};

                if (sceneObj == nullptr) {
                    LOG_ERROR (m_defaultMeshInstanceBatchingInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                                  << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_defaultMeshInstanceBatchingInfo.resource.sceneObj = sceneObj;
            }

            std::vector <MeshInstanceSBO>& getBatchedMeshInstances (void) {
                return m_defaultMeshInstanceBatchingInfo.meta.instances;
            }

            void update (void) {
                auto& sceneObj = m_defaultMeshInstanceBatchingInfo.resource.sceneObj;
                /* Clear previous batched data */
                m_defaultMeshInstanceBatchingInfo.meta.entityToIdxMap.clear();
                m_defaultMeshInstanceBatchingInfo.meta.instances.clear();

                size_t loopIdx = 0;
                for (auto const& entity: m_entities) {
                    auto transformComponent     = sceneObj->getComponent <TransformComponent>     (entity);
                    auto textureIdxLUTComponent = sceneObj->getComponent <TextureIdxLUTComponent> (entity);

                    m_defaultMeshInstanceBatchingInfo.meta.entityToIdxMap[entity] = loopIdx++;
                    MeshInstanceSBO instance;
                    glm::mat4 modelMatrix = transformComponent->createModelMatrix();
                    instance.modelMatrix  = modelMatrix;
                    instance.normalMatrix = glm::mat4 (glm::transpose (glm::inverse (glm::mat3 (modelMatrix))));
                    /* Array copy */
                    textureIdxLUTComponent->copyToTextureIdxLUT (
                        instance.textureIdxLUT
                    );
                    m_defaultMeshInstanceBatchingInfo.meta.instances.push_back (instance);
                }
            }

            void generateReport (void) {
                auto& logObj  = m_defaultMeshInstanceBatchingInfo.resource.logObj;
                size_t rowIdx = 0;

                for (auto const& [entity, idx]: m_defaultMeshInstanceBatchingInfo.meta.entityToIdxMap) {
                    auto& modelMatrix   = m_defaultMeshInstanceBatchingInfo.meta.instances[idx].modelMatrix;
                    auto& normalMatrix  = m_defaultMeshInstanceBatchingInfo.meta.instances[idx].normalMatrix;
                    auto& textureIdxLUT = m_defaultMeshInstanceBatchingInfo.meta.instances[idx].textureIdxLUT;

                    LOG_LITE_INFO (logObj)     << entity << std::endl;
                    LOG_LITE_INFO (logObj)     << "{"    << std::endl;
                    /* Model matrix */
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
                    /* Normal matrix */
                    LOG_LITE_INFO (logObj)     << "\t"   << "[" << std::endl;
                    while (rowIdx < 4) {
                        LOG_LITE_INFO (logObj) << "\t\t" << ALIGN_AND_PAD_C (16) << normalMatrix[rowIdx][0] << ", "
                                                         << ALIGN_AND_PAD_C (16) << normalMatrix[rowIdx][1] << ", "
                                                         << ALIGN_AND_PAD_C (16) << normalMatrix[rowIdx][2] << ", "
                                                         << ALIGN_AND_PAD_C (16) << normalMatrix[rowIdx][3]
                                                         << std::endl;
                        ++rowIdx;
                    }
                    LOG_LITE_INFO (logObj)     << "\t"   << "]" << std::endl;
                    rowIdx = 0;
                    /* Texture idx LUT */
                    LOG_LITE_INFO (logObj)     << "\t"   << "[" << std::endl;
                    while (rowIdx < 8) {
                        LOG_LITE_INFO (logObj) << "\t\t" << ALIGN_AND_PAD_C (12) << textureIdxLUT[0] << ", "
                                                         << ALIGN_AND_PAD_C (12) << textureIdxLUT[1] << ", "
                                                         << ALIGN_AND_PAD_C (12) << textureIdxLUT[2] << ", "
                                                         << ALIGN_AND_PAD_C (12) << textureIdxLUT[3] << ", "
                                                         << ALIGN_AND_PAD_C (12) << textureIdxLUT[4] << ", "
                                                         << ALIGN_AND_PAD_C (12) << textureIdxLUT[5] << ", "
                                                         << ALIGN_AND_PAD_C (12) << textureIdxLUT[6] << ", "
                                                         << ALIGN_AND_PAD_C (12) << textureIdxLUT[7]
                                                         << std::endl;
                        ++rowIdx;
                    }
                    LOG_LITE_INFO (logObj)     << "\t"   << "]" << std::endl;
                    rowIdx = 0;
                    LOG_LITE_INFO (logObj)     << "}"    << std::endl;
                }
            }

            ~SYDefaultMeshInstanceBatching (void) {
                delete m_defaultMeshInstanceBatchingInfo.resource.logObj;
            }
    };
}   // namespace SandBox