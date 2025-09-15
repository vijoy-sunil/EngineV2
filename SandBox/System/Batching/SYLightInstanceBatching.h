#pragma once
#include "../../../Backend/Common.h"
#include "../../../Backend/Scene/SNSystemBase.h"
#include "../../../Backend/Scene/SNImpl.h"
#include "../../../Backend/Log/LGImpl.h"
#include "../../../Backend/Scene/SNType.h"
#include "../../SBComponentType.h"
#include "../../SBRendererType.h"

namespace SandBox {
    class SYLightInstanceBatching: public Scene::SNSystemBase {
        private:
            /* SBO - Storage buffer object */
            struct LightInstanceSBO {
                glm::vec3 position;
                alignas (16) glm::vec3 direction;

                alignas (16) glm::vec3 ambient;
                alignas (16) glm::vec3 diffuse;
                alignas (16) glm::vec3 specular;

                float constant;
                float linear;
                float quadratic;
                float innerRadius;
                float outerRadius;
                float farPlane;

                alignas (16) glm::mat4 viewMatrix;
                glm::mat4 projectionMatrix;
            };

            struct LightInstanceBatchingInfo {
                struct Meta {
                    /* Used for report purpose only */
                    std::unordered_map <Scene::Entity, size_t> entityToIdxMap;

                    std::vector <ActiveLightPC> activeLights;
                    std::vector <LightInstanceSBO> instances;
                    LightTypeOffsetsPC typeOffsets;
                } meta;

                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Log::LGImpl* logObj;
                } resource;
            } m_lightInstanceBatchingInfo;

            /* The textures that make up the cube map are laid out as shown below, with the camera or active light facing
             * forward towards +Z and +X on the right
             *                              +-----------+
             *                              | ^         |
             *                              |     PY    |
             *                              |           |
             *                  +-----------+-----------+-----------+-----------+
             *                  | ^         | ^         | ^         | ^         |
             *                  |     NX    |     PZ    |     PX    |     NZ    |
             *                  |           |           |           |           |
             *                  +-----------+-----------+-----------+-----------+
             *                              | ^         |
             *                              |     NY    |
             *                              |           |
             *                              +-----------+
             * The view matrices created below is to reconstruct the cube map arrangement when the camera or active light
             * is facing forward towards -Z and +X on the left
             *                              +-----------+
             *                              |           |
             *                              |     PY    |
             *                              |         v |
             *                  +-----------+-----------+-----------+-----------+
             *                  | ^         | ^         | ^         | ^         |
             *                  |     PX    |     NZ    |     NX    |     PZ    |
             *                  |           |           |           |           |
             *                  +-----------+-----------+-----------+-----------+
             *                              |           |
             *                              |     NY    |
             *                              |         v |
             *                              +-----------+
            */
            glm::mat4 createViewMatrix (const uint32_t cubeFaceIdx, const glm::vec3 position) {
                glm::mat4 viewMatrix;
                switch (cubeFaceIdx) {
                    case 0: /* PX */
                            viewMatrix = glm::lookAt (position,
                                                      position + glm::vec3 ( 1.0f,  0.0f,  0.0f),
                                                                 glm::vec3 ( 0.0f,  1.0f,  0.0f));
                            break;
                    case 1: /* NX */
                            viewMatrix = glm::lookAt (position,
                                                      position + glm::vec3 (-1.0f,  0.0f,  0.0f),
                                                                 glm::vec3 ( 0.0f,  1.0f,  0.0f));
                            break;
                    case 2: /* PY */
                            viewMatrix = glm::lookAt (position,
                                                      position + glm::vec3 ( 0.0f,  1.0f,  0.0f),
                                                                 glm::vec3 ( 0.0f,  0.0f, -1.0f));
                            break;
                    case 3: /* NY */
                            viewMatrix = glm::lookAt (position,
                                                      position + glm::vec3 ( 0.0f, -1.0f,  0.0f),
                                                                 glm::vec3 ( 0.0f,  0.0f,  1.0f));
                            break;
                    case 4:	/* PZ */
                            viewMatrix = glm::lookAt (position,
                                                      position + glm::vec3 ( 0.0f,  0.0f,  1.0f),
                                                                 glm::vec3 ( 0.0f,  1.0f,  0.0f));
                            break;
                    case 5:	/* NZ */
                            viewMatrix = glm::lookAt (position,
                                                      position + glm::vec3 ( 0.0f,  0.0f, -1.0f),
                                                                 glm::vec3 ( 0.0f,  1.0f,  0.0f));
                            break;
                }
                return viewMatrix;
            }

        public:
            SYLightInstanceBatching (void) {
                m_lightInstanceBatchingInfo = {};

                auto& logObj = m_lightInstanceBatchingInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",     __FILE__);
                logObj->updateLogConfig (Log::LEVEL_TYPE_INFO,    Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_WARNING, Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_ERROR,   Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
            }

            void initLightInstanceBatchingInfo (Scene::SNImpl* sceneObj) {
                auto& meta          = m_lightInstanceBatchingInfo.meta;
                auto& resource      = m_lightInstanceBatchingInfo.resource;

                meta.entityToIdxMap = {};
                meta.activeLights   = {};
                meta.instances      = {};
                meta.typeOffsets    = {};

                if (sceneObj == nullptr) {
                    LOG_ERROR (resource.logObj) << NULL_DEPOBJ_MSG
                                                << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                resource.sceneObj   = sceneObj;
            }

            std::vector <ActiveLightPC>& getBatchedActiveLights (void) {
                return m_lightInstanceBatchingInfo.meta.activeLights;
            }

            std::vector <LightInstanceSBO>& getBatchedLightInstances (void) {
                return m_lightInstanceBatchingInfo.meta.instances;
            }

            LightTypeOffsetsPC* getLightTypeOffsets (void) {
                return &m_lightInstanceBatchingInfo.meta.typeOffsets;
            }

            void update (const float aspectRatio) {
                auto& meta                = m_lightInstanceBatchingInfo.meta;
                auto& sceneObj            = m_lightInstanceBatchingInfo.resource.sceneObj;
                size_t loopIdx            = 0;
                uint32_t sunLightsCount   = 0;
                uint32_t spotLightsCount  = 0;
                uint32_t pointLightsCount = 0;
                /* Clear previous batched data */
                meta.entityToIdxMap.clear();
                meta.activeLights.clear();
                meta.instances.clear();

                for (auto const& entity: m_entities) {
                    auto lightComponent          = sceneObj->getComponent <LightComponent>     (entity);
                    auto transformComponent      = sceneObj->getComponent <TransformComponent> (entity);
                    auto& lightType              = lightComponent->m_lightType;

                    if (lightType == LIGHT_TYPE_SUN)    ++sunLightsCount;
                    if (lightType == LIGHT_TYPE_SPOT)   ++spotLightsCount;
                    if (lightType == LIGHT_TYPE_POINT)  ++pointLightsCount;

                    meta.entityToIdxMap[entity]  = loopIdx++;

                    ActiveLightPC activeLight;
                    activeLight.position         = transformComponent->m_position;
                    activeLight.farPlane         = lightComponent->m_farPlane;
                    activeLight.projectionMatrix = lightComponent->createProjectionMatrix (aspectRatio);

                    if (lightType != LIGHT_TYPE_POINT) {
                        activeLight.viewMatrix   = glm::inverse (transformComponent->createModelMatrix (true));
                        meta.activeLights.push_back (activeLight);
                    }
                    else { for (uint32_t cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx++) {
                        activeLight.viewMatrix   = createViewMatrix (cubeFaceIdx, transformComponent->m_position);
                        meta.activeLights.push_back (activeLight);
                    }}

                    LightInstanceSBO instance;
                    instance.position            = activeLight.position;
                    instance.direction           = transformComponent->getForwardVector();
                    instance.ambient             = lightComponent->m_ambient;
                    instance.diffuse             = lightComponent->m_diffuse;
                    instance.specular            = lightComponent->m_specular;
                    instance.constant            = lightComponent->m_constant;
                    instance.linear              = lightComponent->m_linear;
                    instance.quadratic           = lightComponent->m_quadratic;
                    instance.innerRadius         = lightComponent->m_innerRadius;
                    instance.outerRadius         = lightComponent->m_outerRadius;
                    instance.farPlane            = activeLight.farPlane;
                    instance.viewMatrix          = activeLight.viewMatrix;
                    instance.projectionMatrix    = activeLight.projectionMatrix;
                    meta.instances.push_back (instance);
                }
                /* Note that light entities will be ordered based on light type and batched together as follows
                 *
                 *  Active lights
                 *  +-----------+   +-----------+-----------+   +-----------+-----------+   +-----------+   +-----------+
                 *  |  SUN      |   |  SUN      |  SPOT     |   |  SPOT     |  POINT    |   |  POINT    |   |  POINT    |
                 *  |  0        |...|  n-1      |  0        |...|  n-1      |  0,0      |...|  0,5      |...|  n-1,5    |
                 *  +-----------+   +-----------+-----------+   +-----------+-----------+   +-----------+   +-----------+
                 *                                                          ^                                           ^
                 *                                                          |                           +---------------|
                 *                                                          |                           |
                 *  Instances                                               |                           |
                 *  +-----------+   +-----------+-----------+   +-----------+-----------+   +-----------+
                 *  |  SUN      |   |  SUN      |  SPOT     |   |  SPOT     |  POINT    |   |  POINT    |
                 *  |  0        |...|  n-1      |  0        |...|  n-1      |  0        |...|  n-1      |
                 *  +-----------+   +-----------+-----------+   +-----------+-----------+   +-----------+
                 *                              ^                           ^
                 *                              |                           |
                 *                              Spot lights offset          Point lights offset
                */
                meta.typeOffsets.spotLightsOffset  = sunLightsCount;
                meta.typeOffsets.pointLightsOffset = meta.typeOffsets.spotLightsOffset  + spotLightsCount;
                meta.typeOffsets.lightsCount       = meta.typeOffsets.pointLightsOffset + pointLightsCount;
            }

            void generateReport (void) {
                auto& meta     = m_lightInstanceBatchingInfo.meta;
                auto& resource = m_lightInstanceBatchingInfo.resource;
                auto& logObj   = resource.logObj;

                for (auto const& [entity, idx]: meta.entityToIdxMap) {
                    auto metaComponent  = resource.sceneObj->getComponent <MetaComponent>  (entity);
                    auto lightComponent = resource.sceneObj->getComponent <LightComponent> (entity);
                    auto instance       = meta.instances[idx];

                    LOG_LITE_INFO (logObj)     << metaComponent->m_id              << std::endl;
                    LOG_LITE_INFO (logObj)     << "{"                              << std::endl;

                    LOG_LITE_INFO (logObj)     << "\t"     << "("
                                                           << ALIGN_AND_PAD_C (16) << instance.position.x  << ", "
                                                           << ALIGN_AND_PAD_C (16) << instance.position.y  << ", "
                                                           << ALIGN_AND_PAD_C (16) << instance.position.z
                                                           << ")"
                                                           << std::endl;
                    LOG_LITE_INFO (logObj)     << "\t"     << "("
                                                           << ALIGN_AND_PAD_C (16) << instance.direction.x << ", "
                                                           << ALIGN_AND_PAD_C (16) << instance.direction.y << ", "
                                                           << ALIGN_AND_PAD_C (16) << instance.direction.z
                                                           << ")"
                                                           << std::endl;
                    LOG_LITE_INFO (logObj)     << "\t"     << "("
                                                           << ALIGN_AND_PAD_C (16) << instance.ambient.x   << ", "
                                                           << ALIGN_AND_PAD_C (16) << instance.ambient.y   << ", "
                                                           << ALIGN_AND_PAD_C (16) << instance.ambient.z
                                                           << ")"
                                                           << std::endl;
                    LOG_LITE_INFO (logObj)     << "\t"     << "("
                                                           << ALIGN_AND_PAD_C (16) << instance.diffuse.x   << ", "
                                                           << ALIGN_AND_PAD_C (16) << instance.diffuse.y   << ", "
                                                           << ALIGN_AND_PAD_C (16) << instance.diffuse.z
                                                           << ")"
                                                           << std::endl;
                    LOG_LITE_INFO (logObj)     << "\t"     << "("
                                                           << ALIGN_AND_PAD_C (16) << instance.specular.x  << ", "
                                                           << ALIGN_AND_PAD_C (16) << instance.specular.y  << ", "
                                                           << ALIGN_AND_PAD_C (16) << instance.specular.z
                                                           << ")"
                                                           << std::endl;

                    LOG_LITE_INFO (logObj)     << "\t"     << instance.constant    << std::endl;
                    LOG_LITE_INFO (logObj)     << "\t"     << instance.linear      << std::endl;
                    LOG_LITE_INFO (logObj)     << "\t"     << instance.quadratic   << std::endl;
                    LOG_LITE_INFO (logObj)     << "\t"     << instance.innerRadius << std::endl;
                    LOG_LITE_INFO (logObj)     << "\t"     << instance.outerRadius << std::endl;
                    LOG_LITE_INFO (logObj)     << "\t"     << instance.farPlane    << std::endl;

                    size_t activeLightStartIdx = idx;
                    size_t activeLightEndIdx   = lightComponent->m_lightType != LIGHT_TYPE_POINT ? idx + 1: idx + 6;

                    for (size_t i = activeLightStartIdx; i < activeLightEndIdx; i++) {
                        auto& viewMatrix       = meta.activeLights[i].viewMatrix;
                        auto& projectionMatrix = meta.activeLights[i].projectionMatrix;
                        size_t rowIdx          = 0;

                        LOG_LITE_INFO (logObj) << "\t"     << "{"                  << std::endl;
                        LOG_LITE_INFO (logObj) << "\t\t"   << "["                  << std::endl;
                        while (rowIdx < 4) {
                        LOG_LITE_INFO (logObj) << "\t\t\t" << ALIGN_AND_PAD_C (16) << viewMatrix[rowIdx][0]       << ", "
                                                           << ALIGN_AND_PAD_C (16) << viewMatrix[rowIdx][1]       << ", "
                                                           << ALIGN_AND_PAD_C (16) << viewMatrix[rowIdx][2]       << ", "
                                                           << ALIGN_AND_PAD_C (16) << viewMatrix[rowIdx][3]
                                                           << std::endl;
                        ++rowIdx;
                        }
                        rowIdx = 0;
                        LOG_LITE_INFO (logObj) << "\t\t"   << "]"                  << std::endl;
                        LOG_LITE_INFO (logObj) << "\t\t"   << "["                  << std::endl;
                        while (rowIdx < 4) {
                        LOG_LITE_INFO (logObj) << "\t\t\t" << ALIGN_AND_PAD_C (16) << projectionMatrix[rowIdx][0] << ", "
                                                           << ALIGN_AND_PAD_C (16) << projectionMatrix[rowIdx][1] << ", "
                                                           << ALIGN_AND_PAD_C (16) << projectionMatrix[rowIdx][2] << ", "
                                                           << ALIGN_AND_PAD_C (16) << projectionMatrix[rowIdx][3]
                                                           << std::endl;
                        ++rowIdx;
                        }
                        rowIdx = 0;
                        LOG_LITE_INFO (logObj) << "\t\t"   << "]"                  << std::endl;
                        LOG_LITE_INFO (logObj) << "\t"     << "}"                  << std::endl;
                    }
                    LOG_LITE_INFO (logObj)     << "}"                              << std::endl;
                }
            }

            ~SYLightInstanceBatching (void) {
                delete m_lightInstanceBatchingInfo.resource.logObj;
            }
    };
}   // namespace SandBox