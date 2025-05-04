#pragma once
#include "../../Backend/Common.h"
#include "../../Backend/Scene/SNSystemBase.h"
#include "../../Backend/Scene/SNImpl.h"
#include "../../Backend/Log/LGImpl.h"
#include "../../Backend/Scene/SNType.h"
#include "../SBComponentType.h"
#include "../SBRendererType.h"

namespace SandBox {
    class SYLightInstanceBatching: public Scene::SNSystemBase {
        private:
            /* SBO - Storage buffer object */
            struct LightInstanceSBO {
                glm::vec3 position;                 /* Vec3 must be aligned by 4N (= 16 bytes) */
                alignas (16) glm::vec3 direction;

                alignas (16) glm::vec3 ambient;
                alignas (16) glm::vec3 diffuse;
                alignas (16) glm::vec3 specular;

                float constant;                     /* Scalars must be aligned by N (= 4 bytes given 32 bit floats) */
                float linear;
                float quadratic;
                float innerRadius;
                float outerRadius;
            };

            struct LightInstanceBatchingInfo {
                struct Meta {
                    LightTypeOffsetsPC typeOffsets;
                    /* Used for report purpose only */
                    std::unordered_map <Scene::Entity, size_t> entityToIdxMap;
                    std::vector <LightInstanceSBO> instances;
                } meta;

                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Log::LGImpl* logObj;
                } resource;
            } m_lightInstanceBatchingInfo;

        public:
            SYLightInstanceBatching (void) {
                m_lightInstanceBatchingInfo = {};

                auto& logObj = m_lightInstanceBatchingInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",    __FILE__);
                logObj->updateLogConfig (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
            }

            void initLightInstanceBatchingInfo (Scene::SNImpl* sceneObj) {
                m_lightInstanceBatchingInfo.meta.typeOffsets    = {};
                m_lightInstanceBatchingInfo.meta.entityToIdxMap = {};
                m_lightInstanceBatchingInfo.meta.instances      = {};

                if (sceneObj == nullptr) {
                    LOG_ERROR (m_lightInstanceBatchingInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                            << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_lightInstanceBatchingInfo.resource.sceneObj = sceneObj;
            }

            LightTypeOffsetsPC* getLightTypeOffsets (void) {
                return &m_lightInstanceBatchingInfo.meta.typeOffsets;
            }

            std::vector <LightInstanceSBO>& getBatchedLightInstances (void) {
                return m_lightInstanceBatchingInfo.meta.instances;
            }

            void update (void) {
                auto& typeOffsets = m_lightInstanceBatchingInfo.meta.typeOffsets;
                auto& sceneObj    = m_lightInstanceBatchingInfo.resource.sceneObj;
                /* Clear previous batched data */
                m_lightInstanceBatchingInfo.meta.entityToIdxMap.clear();
                m_lightInstanceBatchingInfo.meta.instances.clear();

                size_t loopIdx                  = 0;
                uint32_t directionalLightsCount = 0;
                uint32_t pointLightsCount       = 0;
                uint32_t spotLightsCount        = 0;
                for (auto const& entity: m_entities) {
                    auto lightComponent     = sceneObj->getComponent <LightComponent>     (entity);
                    auto transformComponent = sceneObj->getComponent <TransformComponent> (entity);
                    auto lightType          = lightComponent->m_type;

                    if (lightType == LIGHT_TYPE_DIRECTIONAL)    ++directionalLightsCount;
                    if (lightType == LIGHT_TYPE_POINT)          ++pointLightsCount;
                    if (lightType == LIGHT_TYPE_SPOT)           ++spotLightsCount;

                    m_lightInstanceBatchingInfo.meta.entityToIdxMap[entity] = loopIdx++;
                    LightInstanceSBO instance;
                    instance.position    = transformComponent->m_position;
                    instance.direction   = transformComponent->createForwardVector();

                    instance.ambient.x   = lightComponent->m_ambient.x;
                    instance.ambient.y   = lightComponent->m_ambient.y;
                    instance.ambient.z   = lightComponent->m_ambient.z;

                    instance.diffuse.x   = lightComponent->m_diffuse.x;
                    instance.diffuse.y   = lightComponent->m_diffuse.y;
                    instance.diffuse.z   = lightComponent->m_diffuse.z;

                    instance.specular.x  = lightComponent->m_specular.x;
                    instance.specular.y  = lightComponent->m_specular.y;
                    instance.specular.z  = lightComponent->m_specular.z;

                    instance.constant    = lightComponent->m_constant;
                    instance.linear      = lightComponent->m_linear;
                    instance.quadratic   = lightComponent->m_quadratic;
                    instance.innerRadius = lightComponent->m_innerRadius;
                    instance.outerRadius = lightComponent->m_outerRadius;

                    m_lightInstanceBatchingInfo.meta.instances.push_back (instance);
                }
                /* Note that light entities will be ordered based on light type and batched together as follows
                 *  +---------------+   +---------------+-----------+   +-----------+-----------+   +-----------+
                 *  |  DIRECTIONAL  |   |  DIRECTIONAL  |  POINT    |   |  POINT    |  SPOT     |   |  SPOT     |
                 *  |  0            |...|  n-1          |  0        |...|  n-1      |  0        |...|  n-1      |
                 *  +---------------+   +---------------+-----------+   +-----------+-----------+   +-----------+
                 *     ^                                   ^                           ^
                 *     |                                   |                           |
                 *     Direction lights offset             Point lights offset         Spot lights offset
                */
                typeOffsets.directionalLightsOffset = 0;
                typeOffsets.pointLightsOffset       = typeOffsets.directionalLightsOffset + directionalLightsCount;
                typeOffsets.spotLightsOffset        = typeOffsets.pointLightsOffset       + pointLightsCount;
                typeOffsets.lightsCount             = typeOffsets.spotLightsOffset        + spotLightsCount;
            }

            void generateReport (void) {
                auto& logObj = m_lightInstanceBatchingInfo.resource.logObj;

                for (auto const& [entity, idx]: m_lightInstanceBatchingInfo.meta.entityToIdxMap) {
                    auto instance = m_lightInstanceBatchingInfo.meta.instances[idx];

                    LOG_LITE_INFO (logObj) << entity << std::endl;
                    LOG_LITE_INFO (logObj) << "{"    << std::endl;
                    LOG_LITE_INFO (logObj) << "\t"   << "("
                                                     << ALIGN_AND_PAD_C (16) << instance.position.x  << ", "
                                                     << ALIGN_AND_PAD_C (16) << instance.position.y  << ", "
                                                     << ALIGN_AND_PAD_C (16) << instance.position.z
                                                     << ")"
                                                     << std::endl;
                    LOG_LITE_INFO (logObj) << "\t"   << "("
                                                     << ALIGN_AND_PAD_C (16) << instance.direction.x << ", "
                                                     << ALIGN_AND_PAD_C (16) << instance.direction.y << ", "
                                                     << ALIGN_AND_PAD_C (16) << instance.direction.z
                                                     << ")"
                                                     << std::endl;
                    LOG_LITE_INFO (logObj) << "\t"   << "("
                                                     << ALIGN_AND_PAD_C (16) << instance.ambient.x   << ", "
                                                     << ALIGN_AND_PAD_C (16) << instance.ambient.y   << ", "
                                                     << ALIGN_AND_PAD_C (16) << instance.ambient.z
                                                     << ")"
                                                     << std::endl;
                    LOG_LITE_INFO (logObj) << "\t"   << "("
                                                     << ALIGN_AND_PAD_C (16) << instance.diffuse.x   << ", "
                                                     << ALIGN_AND_PAD_C (16) << instance.diffuse.y   << ", "
                                                     << ALIGN_AND_PAD_C (16) << instance.diffuse.z
                                                     << ")"
                                                     << std::endl;
                    LOG_LITE_INFO (logObj) << "\t"   << "("
                                                     << ALIGN_AND_PAD_C (16) << instance.specular.x  << ", "
                                                     << ALIGN_AND_PAD_C (16) << instance.specular.y  << ", "
                                                     << ALIGN_AND_PAD_C (16) << instance.specular.z
                                                     << ")"
                                                     << std::endl;

                    LOG_LITE_INFO (logObj) << "\t"   << instance.constant    << std::endl;
                    LOG_LITE_INFO (logObj) << "\t"   << instance.linear      << std::endl;
                    LOG_LITE_INFO (logObj) << "\t"   << instance.quadratic   << std::endl;
                    LOG_LITE_INFO (logObj) << "\t"   << instance.innerRadius << std::endl;
                    LOG_LITE_INFO (logObj) << "\t"   << instance.outerRadius << std::endl;
                    LOG_LITE_INFO (logObj) << "}"    << std::endl;
                }
            }

            ~SYLightInstanceBatching (void) {
                delete m_lightInstanceBatchingInfo.resource.logObj;
            }
    };
}   // namespace SandBox