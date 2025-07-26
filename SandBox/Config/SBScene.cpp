#include "../../Backend/Common.h"
#include "../../Backend/Scene/SNImpl.h"
#include "../System/Loading/SYMeshLoading.h"
#include "../System/Batching/SYMeshBatching.h"
#include "../System/Batching/SYStdMeshInstanceBatching.h"
#include "../System/Batching/SYWireMeshInstanceBatching.h"
#include "../System/Batching/SYLightInstanceBatching.h"
#include "../System/Controller/SYCameraController.h"
#include "../System/Rendering/SYShadowRendering.h"
#include "../System/Rendering/SYShadowCubeRendering.h"
#include "../System/Rendering/SYGDefaultRendering.h"
#include "../System/Rendering/SYLightRendering.h"
#include "../System/Rendering/SYWireRendering.h"
#include "../System/Rendering/SYSkyBoxRendering.h"
#include "../System/Rendering/SYFDefaultRendering.h"
#include "../System/Rendering/SYDebugRendering.h"
#include "../SBImpl.h"
#include "../../Backend/Scene/SNType.h"
#include "../SBComponentType.h"

/* Short hand macros */
#define R_45        glm::radians ( 45.0f)
#define R_90        glm::radians ( 90.0f)
#define R_180       glm::radians (180.0f)

namespace SandBox {
    void SBImpl::configScene (void) {
        auto& shadowImageWidth     = m_sandBoxInfo.meta.shadowImageWidth;
        auto& shadowImageHeight    = m_sandBoxInfo.meta.shadowImageHeight;
        auto& skyBoxEntity         = m_sandBoxInfo.meta.skyBoxEntity;
        auto& entityFamilyInfoPool = m_sandBoxInfo.meta.entityFamilyInfoPool;
        auto& sceneObj             = m_sandBoxInfo.resource.sceneObj;
        auto& stdTexturePoolObj    = m_sandBoxInfo.resource.stdTexturePoolObj;

        std::vector <Scene::Entity> srcTextureIdxLUTComponentEntities;

        {   /* Register components */
            sceneObj->registerComponent <MetaComponent>();
            sceneObj->registerComponent <MeshComponent>();
            sceneObj->registerComponent <LightComponent>();
            sceneObj->registerComponent <CameraComponent>();
            sceneObj->registerComponent <TransformComponent>();
            sceneObj->registerComponent <TextureIdxLUTComponent>();
            sceneObj->registerComponent <ColorComponent>();
            sceneObj->registerComponent <RenderComponent>();
            sceneObj->registerComponent <StdTagComponent>();
            sceneObj->registerComponent <StdAlphaTagComponent>();
            sceneObj->registerComponent <WireTagComponent>();
            sceneObj->registerComponent <SkyBoxTagComponent>();
        }
        {   /* Register systems */
            auto meshLoadingObj              = sceneObj->registerSystem <SYMeshLoading>();
            auto meshBatchingObj             = sceneObj->registerSystem <SYMeshBatching>();
            auto stdMeshInstanceBatchingObj  = sceneObj->registerSystem <SYStdMeshInstanceBatching>();
            auto wireMeshInstanceBatchingObj = sceneObj->registerSystem <SYWireMeshInstanceBatching>();
            auto lightInstanceBatchingObj    = sceneObj->registerSystem <SYLightInstanceBatching>();
            auto cameraControllerObj         = sceneObj->registerSystem <SYCameraController>();
            auto shadowRenderingObj          = sceneObj->registerSystem <SYShadowRendering>();
            auto shadowCubeRenderingObj      = sceneObj->registerSystem <SYShadowCubeRendering>();
            auto gDefaultRenderingObj        = sceneObj->registerSystem <SYGDefaultRendering>();
            auto lightRenderingObj           = sceneObj->registerSystem <SYLightRendering>();
            auto wireRenderingObj            = sceneObj->registerSystem <SYWireRendering>();
            auto skyBoxRenderingObj          = sceneObj->registerSystem <SYSkyBoxRendering>();
            auto fDefaultRenderingObj        = sceneObj->registerSystem <SYFDefaultRendering>();
            auto debugRenderingObj           = sceneObj->registerSystem <SYDebugRendering>();
            /* Set system signature */
            {   /* Mesh loading system */
                meshLoadingObj->initMeshLoadingInfo (sceneObj, stdTexturePoolObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <MeshComponent>());
                systemSignature.set (sceneObj->getComponentType <TextureIdxLUTComponent>());

                sceneObj->setSystemSignature <SYMeshLoading> (systemSignature);
            }
            {   /* Mesh batching system */
                meshBatchingObj->initMeshBatchingInfo (sceneObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <MetaComponent>());
                systemSignature.set (sceneObj->getComponentType <MeshComponent>());
                systemSignature.set (sceneObj->getComponentType <RenderComponent>());

                sceneObj->setSystemSignature <SYMeshBatching> (systemSignature);
            }
            {   /* Std mesh instance batching system */
                stdMeshInstanceBatchingObj->initStdMeshInstanceBatchingInfo (sceneObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <MetaComponent>());
                systemSignature.set (sceneObj->getComponentType <TransformComponent>());
                systemSignature.set (sceneObj->getComponentType <TextureIdxLUTComponent>());

                sceneObj->setSystemSignature <SYStdMeshInstanceBatching> (systemSignature);
            }
            {   /* Wire mesh instance batching system */
                wireMeshInstanceBatchingObj->initWireMeshInstanceBatchingInfo (sceneObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <TransformComponent>());
                systemSignature.set (sceneObj->getComponentType <ColorComponent>());

                sceneObj->setSystemSignature <SYWireMeshInstanceBatching> (systemSignature);
            }
            {   /* Light instance batching system */
                lightInstanceBatchingObj->initLightInstanceBatchingInfo (sceneObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <LightComponent>());
                systemSignature.set (sceneObj->getComponentType <TransformComponent>());

                sceneObj->setSystemSignature <SYLightInstanceBatching> (systemSignature);
            }
            {   /* Camera controller system */
                /* Init method called post-renderer-config */
                static_cast <void> (cameraControllerObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <CameraComponent>());
                systemSignature.set (sceneObj->getComponentType <TransformComponent>());

                sceneObj->setSystemSignature <SYCameraController> (systemSignature);
            }
            {   /* Shadow rendering system */
                static_cast <void> (shadowRenderingObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <RenderComponent>());
                systemSignature.set (sceneObj->getComponentType <StdTagComponent>());

                sceneObj->setSystemSignature <SYShadowRendering> (systemSignature);
            }
            {   /* Shadow cube rendering system */
                static_cast <void> (shadowCubeRenderingObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <RenderComponent>());
                systemSignature.set (sceneObj->getComponentType <StdTagComponent>());

                sceneObj->setSystemSignature <SYShadowCubeRendering> (systemSignature);
            }
            {   /* G default rendering system */
                static_cast <void> (gDefaultRenderingObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <RenderComponent>());
                systemSignature.set (sceneObj->getComponentType <StdTagComponent>());

                sceneObj->setSystemSignature <SYGDefaultRendering> (systemSignature);
            }
            {   /* Light rendering system */
                static_cast <void> (lightRenderingObj);
            }
            {   /* Wire rendering system */
                static_cast <void> (wireRenderingObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <RenderComponent>());
                systemSignature.set (sceneObj->getComponentType <WireTagComponent>());

                sceneObj->setSystemSignature <SYWireRendering> (systemSignature);
            }
            {   /* Sky box rendering system */
                static_cast <void> (skyBoxRenderingObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <RenderComponent>());
                systemSignature.set (sceneObj->getComponentType <SkyBoxTagComponent>());

                sceneObj->setSystemSignature <SYSkyBoxRendering> (systemSignature);
            }
            {   /* F default rendering system */
                static_cast <void> (fDefaultRenderingObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <RenderComponent>());
                systemSignature.set (sceneObj->getComponentType <StdAlphaTagComponent>());

                sceneObj->setSystemSignature <SYFDefaultRendering> (systemSignature);
            }
            {   /* Debug rendering system */
                static_cast <void> (debugRenderingObj);
            }
        }

        /* Entity ordering
         *  +-------+-------+-------+-------+-------+-------+-------+-------+
         *  |  A,0  |  A,1  |  A,2  |  A,3  |  B,0  |  C,0  |  C,1  |  C,2  | ...
         *  +-------+-------+-------+-------+-------+-------+-------+-------+
         *      |       |               |       |       |       |       |
         *      v       +---------------+       v       v       +-------+
         *    Parent        Children          Parent  Parent     Children
        */
        {   /* Entity   [DEBUG_CUBE_?] */
            Scene::Entity parentEntity;
            auto instanceTransforms = std::vector <std::pair <glm::vec3, glm::vec3>> {
                {{34.0f, 0.5f, 34.0f}, {0.0f, 0.0f, 0.0f}},     /* Parent transform */
                {{36.0f, 0.5f, 34.0f}, {0.0f, R_90, 0.0f}},     /* Yaw              */
                {{38.0f, 0.5f, 34.0f}, {R_90, 0.0f, 0.0f}},     /* Pitch            */
                {{38.0f, 0.5f, 36.0f}, {0.0f, 0.0f, R_90}},     /* Roll             */
                {{36.0f, 0.5f, 36.0f}, {R_90, R_90, 0.0f}},     /* Yaw->Pitch       */
                {{34.0f, 0.5f, 36.0f}, {0.0f, R_90, R_90}},     /* Yaw->Roll        */
                {{34.0f, 0.5f, 38.0f}, {R_90, 0.0f, R_90}},     /* Pitch->Roll      */
                {{36.0f, 0.5f, 38.0f}, {R_90, R_90, R_90}}      /* Yaw->Pitch->Roll */
            };
            for (size_t i = 0; i < instanceTransforms.size(); i++) {
                auto entity = sceneObj->addEntity();

                sceneObj->addComponent (entity, MetaComponent (
                    "DEBUG_CUBE_" + std::to_string (i),
                    TAG_TYPE_STD
                ));
                sceneObj->addComponent (entity, TransformComponent (
                    instanceTransforms[i].first,
                    instanceTransforms[i].second
                ));
                sceneObj->addComponent (entity, TextureIdxLUTComponent());

                if (i == 0) {   /* Parent entity */
                    parentEntity = entity;
                    entityFamilyInfoPool[parentEntity] = {};
                    srcTextureIdxLUTComponentEntities.push_back (parentEntity);

                    sceneObj->addComponent (entity, MeshComponent());
                    sceneObj->addComponent (entity, RenderComponent (
                        static_cast <uint32_t> (instanceTransforms.size())
                    ));
                    sceneObj->addComponent (entity, StdTagComponent());
                }
                else
                    entityFamilyInfoPool[parentEntity].push_back (entity);
            }
        }
        {   /* Entity   [DEBUG_SPHERE_?] */
            Scene::Entity parentEntity;
            auto instanceTransforms = std::vector <std::pair <glm::vec3, glm::vec3>> {
                {{26.0f, 0.5f, 36.0f}, {0.0f, 0.0f, 0.0f}}
            };
            for (size_t i = 0; i < instanceTransforms.size(); i++) {
                auto entity = sceneObj->addEntity();

                sceneObj->addComponent (entity, MetaComponent (
                    "DEBUG_SPHERE_" + std::to_string (i),
                    TAG_TYPE_STD
                ));
                sceneObj->addComponent (entity, TransformComponent (
                    instanceTransforms[i].first,
                    instanceTransforms[i].second
                ));
                sceneObj->addComponent (entity, TextureIdxLUTComponent());

                if (i == 0) {   /* Parent entity */
                    parentEntity = entity;
                    entityFamilyInfoPool[parentEntity] = {};
                    srcTextureIdxLUTComponentEntities.push_back (parentEntity);

                    sceneObj->addComponent (entity, MeshComponent (
                        "Asset/Model/Debug_Sphere.obj",
                        "Asset/Model/"
                    ));
                    sceneObj->addComponent (entity, RenderComponent (
                        static_cast <uint32_t> (instanceTransforms.size())
                    ));
                    sceneObj->addComponent (entity, StdTagComponent());
                }
                else
                    entityFamilyInfoPool[parentEntity].push_back (entity);
            }
        }
        {   /* Entity   [SKY_BOX] */
            auto entity = sceneObj->addEntity();
            sceneObj->addComponent (entity, MetaComponent (
                "SKY_BOX",
                TAG_TYPE_SKY_BOX
            ));
            sceneObj->addComponent (entity, MeshComponent (
                {
                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f, -0.5f,  0.5f}}, {0, 0, 0, 0}},          /* Vertex 0 */
                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f, -0.5f,  0.5f}}, {0, 0, 0, 0}},          /* Vertex 1 */
                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f,  0.5f,  0.5f}}, {0, 0, 0, 0}},          /* Vertex 2 */
                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f,  0.5f,  0.5f}}, {0, 0, 0, 0}},          /* Vertex 3 */

                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f, -0.5f, -0.5f}}, {0, 0, 0, 0}},          /* Vertex 4 */
                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f, -0.5f, -0.5f}}, {0, 0, 0, 0}},          /* Vertex 5 */
                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f,  0.5f, -0.5f}}, {0, 0, 0, 0}},          /* Vertex 6 */
                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f,  0.5f, -0.5f}}, {0, 0, 0, 0}}           /* Vertex 7 */
                },
                {
                    0, 1, 2, 0, 2, 3,
                    1, 5, 6, 1, 6, 2,
                    5, 4, 7, 5, 7, 6,
                    4, 0, 3, 4, 3, 7,
                    4, 5, 1, 4, 1, 0,
                    3, 2, 6, 3, 6, 7
                }
            ));
            sceneObj->addComponent (entity, TransformComponent());
            sceneObj->addComponent (entity, RenderComponent());
            sceneObj->addComponent (entity, SkyBoxTagComponent());
            /* Save sky box entity */
            skyBoxEntity = entity;
            /* No children */
            entityFamilyInfoPool[entity] = {};
        }
        {   /* Entity   [GROUND_PLANE] */
            auto entity = sceneObj->addEntity();
            sceneObj->addComponent (entity, MetaComponent (
                "GROUND_PLANE",
                TAG_TYPE_STD
            ));
            sceneObj->addComponent (entity, MeshComponent (
                "Asset/Model/Ground_Plane.obj",
                "Asset/Model/"
            ));
            sceneObj->addComponent (entity, TransformComponent());
            sceneObj->addComponent (entity, TextureIdxLUTComponent());
            sceneObj->addComponent (entity, RenderComponent());
            sceneObj->addComponent (entity, StdTagComponent());
            /* No children */
            entityFamilyInfoPool[entity] = {};
        }
        {   /* Entity   [CHASSIS] */
            auto entity = sceneObj->addEntity();
            sceneObj->addComponent (entity, MetaComponent (
                "CHASSIS",
                TAG_TYPE_STD
            ));
            sceneObj->addComponent (entity, MeshComponent (
                "Asset/Model/Chassis.obj",
                "Asset/Model/"
            ));
            sceneObj->addComponent (entity, TransformComponent (
                {16.0f, 0.3f, 36.0f},
                { 0.0f, 0.0f,  0.0f}
            ));
            sceneObj->addComponent (entity, TextureIdxLUTComponent());
            sceneObj->addComponent (entity, RenderComponent());
            sceneObj->addComponent (entity, StdTagComponent());
            /* No children */
            entityFamilyInfoPool[entity] = {};
        }
        {   /* Entity   [TIRE_?] */
            Scene::Entity parentEntity;
            auto instanceTransforms = std::vector <std::pair <glm::vec3, glm::vec3>> {
                {{15.4f, 0.1f, 37.0f}, {0.0f, 0.0f, R_90}},
                {{16.6f, 0.1f, 37.0f}, {0.0f, 0.0f, R_90}},
                {{16.6f, 0.1f, 35.0f}, {0.0f, 0.0f, R_90}},
                {{15.4f, 0.1f, 35.0f}, {0.0f, 0.0f, R_90}}
            };
            for (size_t i = 0; i < instanceTransforms.size(); i++) {
                auto entity = sceneObj->addEntity();

                sceneObj->addComponent (entity, MetaComponent (
                    "TIRE_" + std::to_string (i),
                    TAG_TYPE_STD
                ));
                sceneObj->addComponent (entity, TransformComponent (
                    instanceTransforms[i].first,
                    instanceTransforms[i].second
                ));
                sceneObj->addComponent (entity, TextureIdxLUTComponent());

                if (i == 0) {   /* Parent entity */
                    parentEntity = entity;
                    entityFamilyInfoPool[parentEntity] = {};
                    srcTextureIdxLUTComponentEntities.push_back (parentEntity);

                    sceneObj->addComponent (entity, MeshComponent (
                        "Asset/Model/Tire.obj",
                        "Asset/Model/"
                    ));
                    sceneObj->addComponent (entity, RenderComponent (
                        static_cast <uint32_t> (instanceTransforms.size())
                    ));
                    sceneObj->addComponent (entity, StdTagComponent());
                }
                else
                    entityFamilyInfoPool[parentEntity].push_back (entity);
            }
        }
        {   /* Entity   [LOW_RAMP_?] */
            Scene::Entity parentEntity;
            auto instanceTransforms = std::vector <std::pair <glm::vec3, glm::vec3>> {
                {{-20.0f, 0.0f,  20.0f}, {0.0f,  0.0f, 0.0f}},
                {{-20.0f, 0.0f, -10.0f}, {0.0f, R_180, 0.0f}}
            };
            for (size_t i = 0; i < instanceTransforms.size(); i++) {
                auto entity = sceneObj->addEntity();

                sceneObj->addComponent (entity, MetaComponent (
                    "LOW_RAMP_" + std::to_string (i),
                    TAG_TYPE_STD
                ));
                sceneObj->addComponent (entity, TransformComponent (
                    instanceTransforms[i].first,
                    instanceTransforms[i].second
                ));
                sceneObj->addComponent (entity, TextureIdxLUTComponent());

                if (i == 0) {   /* Parent entity */
                    parentEntity = entity;
                    entityFamilyInfoPool[parentEntity] = {};
                    srcTextureIdxLUTComponentEntities.push_back (parentEntity);

                    sceneObj->addComponent (entity, MeshComponent (
                        "Asset/Model/Low_Ramp.obj",
                        "Asset/Model/"
                    ));
                    sceneObj->addComponent (entity, RenderComponent (
                        static_cast <uint32_t> (instanceTransforms.size())
                    ));
                    sceneObj->addComponent (entity, StdTagComponent());
                }
                else
                    entityFamilyInfoPool[parentEntity].push_back (entity);
            }
        }
        {   /* Entity   [MED_RAMP_?] */
            Scene::Entity parentEntity;
            auto instanceTransforms = std::vector <std::pair <glm::vec3, glm::vec3>> {
                {{-10.0f, 0.0f, -20.0f}, {0.0f, -R_90, 0.0f}}
            };
            for (size_t i = 0; i < instanceTransforms.size(); i++) {
                auto entity = sceneObj->addEntity();

                sceneObj->addComponent (entity, MetaComponent (
                    "MED_RAMP_" + std::to_string (i),
                    TAG_TYPE_STD
                ));
                sceneObj->addComponent (entity, TransformComponent (
                    instanceTransforms[i].first,
                    instanceTransforms[i].second
                ));
                sceneObj->addComponent (entity, TextureIdxLUTComponent());

                if (i == 0) {   /* Parent entity */
                    parentEntity = entity;
                    entityFamilyInfoPool[parentEntity] = {};
                    srcTextureIdxLUTComponentEntities.push_back (parentEntity);

                    sceneObj->addComponent (entity, MeshComponent (
                        "Asset/Model/Med_Ramp.obj",
                        "Asset/Model/"
                    ));
                    sceneObj->addComponent (entity, RenderComponent (
                        static_cast <uint32_t> (instanceTransforms.size())
                    ));
                    sceneObj->addComponent (entity, StdTagComponent());
                }
                else
                    entityFamilyInfoPool[parentEntity].push_back (entity);
            }
        }
        {   /* Entity   [HIGH_RAMP_?] */
            Scene::Entity parentEntity;
            auto instanceTransforms = std::vector <std::pair <glm::vec3, glm::vec3>> {
                {{-10.0f, 0.0f, -30.0f}, {0.0f, -R_90, 0.0f}}
            };
            for (size_t i = 0; i < instanceTransforms.size(); i++) {
                auto entity = sceneObj->addEntity();

                sceneObj->addComponent (entity, MetaComponent (
                    "HIGH_RAMP_" + std::to_string (i),
                    TAG_TYPE_STD
                ));
                sceneObj->addComponent (entity, TransformComponent (
                    instanceTransforms[i].first,
                    instanceTransforms[i].second
                ));
                sceneObj->addComponent (entity, TextureIdxLUTComponent());

                if (i == 0) {   /* Parent entity */
                    parentEntity = entity;
                    entityFamilyInfoPool[parentEntity] = {};
                    srcTextureIdxLUTComponentEntities.push_back (parentEntity);

                    sceneObj->addComponent (entity, MeshComponent (
                        "Asset/Model/High_Ramp.obj",
                        "Asset/Model/"
                    ));
                    sceneObj->addComponent (entity, RenderComponent (
                        static_cast <uint32_t> (instanceTransforms.size())
                    ));
                    sceneObj->addComponent (entity, StdTagComponent());
                }
                else
                    entityFamilyInfoPool[parentEntity].push_back (entity);
            }
        }
        {   /* Entity   [SPEED_BUMP_?] */
            Scene::Entity parentEntity;
            auto instanceTransforms = std::vector <std::pair <glm::vec3, glm::vec3>> {
                {{20.0f, 0.0f, 20.0f}, {0.0f, -R_45, 0.0f}}
            };
            for (size_t i = 0; i < instanceTransforms.size(); i++) {
                auto entity = sceneObj->addEntity();

                sceneObj->addComponent (entity, MetaComponent (
                    "SPEED_BUMP_" + std::to_string (i),
                    TAG_TYPE_STD
                ));
                sceneObj->addComponent (entity, TransformComponent (
                    instanceTransforms[i].first,
                    instanceTransforms[i].second
                ));
                sceneObj->addComponent (entity, TextureIdxLUTComponent());

                if (i == 0) {   /* Parent entity */
                    parentEntity = entity;
                    entityFamilyInfoPool[parentEntity] = {};
                    srcTextureIdxLUTComponentEntities.push_back (parentEntity);

                    sceneObj->addComponent (entity, MeshComponent (
                        "Asset/Model/Speed_Bump.obj",
                        "Asset/Model/"
                    ));
                    sceneObj->addComponent (entity, RenderComponent (
                        static_cast <uint32_t> (instanceTransforms.size())
                    ));
                    sceneObj->addComponent (entity, StdTagComponent());
                }
                else
                    entityFamilyInfoPool[parentEntity].push_back (entity);
            }
        }
        {   /* Entity   [TRAFFIC_CONE_?] */
            Scene::Entity parentEntity;
            auto instanceTransforms = std::vector <std::pair <glm::vec3, glm::vec3>> {
                {{ -23.0f, 0.0f,   20.0f}, {0.0f, 0.0f, 0.0f}},
                {{ -17.0f, 0.0f,   20.0f}, {0.0f, 0.0f, 0.0f}},
                {{ -17.0f, 0.0f,  -10.0f}, {0.0f, 0.0f, 0.0f}},
                {{ -23.0f, 0.0f,  -10.0f}, {0.0f, 0.0f, 0.0f}},

                {{ -10.0f, 0.0f,  -23.0f}, {0.0f, 0.0f, 0.0f}},
                {{ -10.0f, 0.0f,  -17.0f}, {0.0f, 0.0f, 0.0f}},

                {{ -10.0f, 0.0f,  -33.0f}, {0.0f, 0.0f, 0.0f}},
                {{ -10.0f, 0.0f,  -27.0f}, {0.0f, 0.0f, 0.0f}},

                {{17.879f, 0.0f, 17.879f}, {0.0f, 0.0f, 0.0f}},
                {{22.121f, 0.0f, 22.121f}, {0.0f, 0.0f, 0.0f}},
                {{30.429f, 0.0f, 13.813f}, {0.0f, 0.0f, 0.0f}},
                {{26.187f, 0.0f,  9.570f}, {0.0f, 0.0f, 0.0f}},

                {{  13.0f, 0.0f,   33.0f}, {0.0f, 0.0f, 0.0f}},
                {{  13.0f, 0.0f,   35.0f}, {0.0f, 0.0f, 0.0f}},
                {{  13.0f, 0.0f,   37.0f}, {0.0f, 0.0f, 0.0f}},
                {{  13.0f, 0.0f,   39.0f}, {0.0f, 0.0f, 0.0f}},
                {{  15.0f, 0.0f,   39.0f}, {0.0f, 0.0f, 0.0f}},
                {{  17.0f, 0.0f,   39.0f}, {0.0f, 0.0f, 0.0f}},
                {{  19.0f, 0.0f,   39.0f}, {0.0f, 0.0f, 0.0f}},
                {{  19.0f, 0.0f,   37.0f}, {0.0f, 0.0f, 0.0f}},
                {{  19.0f, 0.0f,   35.0f}, {0.0f, 0.0f, 0.0f}},
                {{  19.0f, 0.0f,   33.0f}, {0.0f, 0.0f, 0.0f}}
            };
            for (size_t i = 0; i < instanceTransforms.size(); i++) {
                auto entity = sceneObj->addEntity();

                sceneObj->addComponent (entity, MetaComponent (
                    "TRAFFIC_CONE_" + std::to_string (i),
                    TAG_TYPE_STD
                ));
                sceneObj->addComponent (entity, TransformComponent (
                    instanceTransforms[i].first,
                    instanceTransforms[i].second
                ));
                sceneObj->addComponent (entity, TextureIdxLUTComponent());

                if (i == 0) {   /* Parent entity */
                    parentEntity = entity;
                    entityFamilyInfoPool[parentEntity] = {};
                    srcTextureIdxLUTComponentEntities.push_back (parentEntity);

                    sceneObj->addComponent (entity, MeshComponent (
                        "Asset/Model/Traffic_Cone.obj",
                        "Asset/Model/"
                    ));
                    sceneObj->addComponent (entity, RenderComponent (
                        static_cast <uint32_t> (instanceTransforms.size())
                    ));
                    sceneObj->addComponent (entity, StdTagComponent());
                }
                else
                    entityFamilyInfoPool[parentEntity].push_back (entity);
            }
        }

        {   /* Entity   [SUN_LIGHT_?] */
            Scene::Entity parentEntity;
            auto instanceTransforms = std::vector <std::pair <glm::vec3, glm::vec3>> {
                {{0.0f, 7.0f, -7.0f}, {-R_45, R_180, 0.0f}}
            };
            for (size_t i = 0; i < instanceTransforms.size(); i++) {
                auto entity = sceneObj->addEntity();

                sceneObj->addComponent (entity, MetaComponent (
                    "SUN_LIGHT_" + std::to_string (i),
                    TAG_TYPE_WIRE
                ));
                sceneObj->addComponent (entity, LightComponent());
                sceneObj->addComponent (entity, TransformComponent (
                    instanceTransforms[i].first,
                    instanceTransforms[i].second,
                    {0.5f, 0.5f, 0.5f}
                ));
                sceneObj->addComponent (entity, ColorComponent (
                    {1.0f, 0.0f, 0.0f, 1.0f}
                ));

                if (i == 0) {   /* Parent entity */
                    parentEntity = entity;
                    entityFamilyInfoPool[parentEntity] = {};

                    sceneObj->addComponent (entity, MeshComponent (
                        {
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f, -0.5f,  0.1f}}, {0, 0, 0, 0}},      /* Vertex 0  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f, -0.5f,  0.1f}}, {0, 0, 0, 0}},      /* Vertex 1  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f,  0.5f,  0.1f}}, {0, 0, 0, 0}},      /* Vertex 2  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f,  0.5f,  0.1f}}, {0, 0, 0, 0}},      /* Vertex 3  */

                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f, -0.5f,  0.0f}}, {0, 0, 0, 0}},      /* Vertex 4  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f, -0.5f,  0.0f}}, {0, 0, 0, 0}},      /* Vertex 5  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f,  0.5f,  0.0f}}, {0, 0, 0, 0}},      /* Vertex 6  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f,  0.5f,  0.0f}}, {0, 0, 0, 0}},      /* Vertex 7  */

                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.1f, -0.1f,  0.0f}}, {0, 0, 0, 0}},      /* Vertex 8  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.1f, -0.1f,  0.0f}}, {0, 0, 0, 0}},      /* Vertex 9  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.1f,  0.1f,  0.0f}}, {0, 0, 0, 0}},      /* Vertex 10 */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.1f,  0.1f,  0.0f}}, {0, 0, 0, 0}},      /* Vertex 11 */

                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f, -0.5f, -0.1f}}, {0, 0, 0, 0}},      /* Vertex 12 */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f, -0.5f, -0.1f}}, {0, 0, 0, 0}},      /* Vertex 13 */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f,  0.5f, -0.1f}}, {0, 0, 0, 0}},      /* Vertex 14 */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f,  0.5f, -0.1f}}, {0, 0, 0, 0}},      /* Vertex 15 */

                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.0f,  0.0f,  0.0f}}, {0, 0, 0, 0}},      /* Vertex 16 */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.0f,  0.0f, -3.0f}}, {0, 0, 0, 0}}       /* Vertex 17 */
                        },
                        {
                             0,  1,  1,  2,  2,  3,  3,  0,
                             4,  5,  5,  6,  6,  7,  7,  4,
                             8,  9,  9, 10, 10, 11, 11,  8,
                            12, 13, 13, 14, 14, 15, 15, 12,
                            16, 17
                        }
                    ));
                    sceneObj->addComponent (entity, RenderComponent (
                        static_cast <uint32_t> (instanceTransforms.size())
                    ));
                    sceneObj->addComponent (entity, WireTagComponent());
                }
                else
                    entityFamilyInfoPool[parentEntity].push_back (entity);
            }
        }
        {   /* Entity   [SPOT_LIGHT_?] */
            Scene::Entity parentEntity;
            auto instanceTransforms = std::vector <std::pair <glm::vec3, glm::vec3>> {
                {{36.0f, 7.0f, 36.0f}, {-R_90, 0.0f, 0.0f}},
                {{26.0f, 7.0f, 36.0f}, {-R_90, 0.0f, 0.0f}},
                {{16.0f, 7.0f, 36.0f}, {-R_90, 0.0f, 0.0f}}
            };
            for (size_t i = 0; i < instanceTransforms.size(); i++) {
                auto entity = sceneObj->addEntity();

                sceneObj->addComponent (entity, MetaComponent (
                    "SPOT_LIGHT_" + std::to_string (i),
                    TAG_TYPE_WIRE
                ));
                sceneObj->addComponent (entity, LightComponent (
                    LIGHT_TYPE_SPOT,
                    {0.0f, 0.0f, 0.0f},
                    {1.0f, 1.0f, 1.0f},
                    {1.0f, 1.0f, 1.0f},
                    1.0f,
                    0.045f,
                    0.0075f,
                    glm::radians (20.0f),
                    glm::radians (30.0f),
                    0.01f,
                    100.0f
                ));
                sceneObj->addComponent (entity, TransformComponent (
                    instanceTransforms[i].first,
                    instanceTransforms[i].second,
                    {0.5f, 0.5f, 0.5f}
                ));
                sceneObj->addComponent (entity, ColorComponent (
                    {1.0f, 0.0f, 0.0f, 1.0f}
                ));

                if (i == 0) {   /* Parent entity */
                    parentEntity = entity;
                    entityFamilyInfoPool[parentEntity] = {};

                    sceneObj->addComponent (entity, MeshComponent (
                        {
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.1f, -0.1f,  0.0f}}, {0, 0, 0, 0}},      /* Vertex 0  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.1f, -0.1f,  0.0f}}, {0, 0, 0, 0}},      /* Vertex 1  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.1f,  0.1f,  0.0f}}, {0, 0, 0, 0}},      /* Vertex 2  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.1f,  0.1f,  0.0f}}, {0, 0, 0, 0}},      /* Vertex 3  */

                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f, -0.5f, -1.0f}}, {0, 0, 0, 0}},      /* Vertex 4  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f, -0.5f, -1.0f}}, {0, 0, 0, 0}},      /* Vertex 5  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f,  0.5f, -1.0f}}, {0, 0, 0, 0}},      /* Vertex 6  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f,  0.5f, -1.0f}}, {0, 0, 0, 0}},      /* Vertex 7  */

                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.6f, -0.6f, -1.0f}}, {0, 0, 0, 0}},      /* Vertex 8  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.6f, -0.6f, -1.0f}}, {0, 0, 0, 0}},      /* Vertex 9  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.6f,  0.6f, -1.0f}}, {0, 0, 0, 0}},      /* Vertex 10 */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.6f,  0.6f, -1.0f}}, {0, 0, 0, 0}},      /* Vertex 11 */

                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.0f,  0.0f,  0.0f}}, {0, 0, 0, 0}},      /* Vertex 12 */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.0f,  0.0f, -3.0f}}, {0, 0, 0, 0}}       /* Vertex 13 */
                        },
                        {
                             0,  1, 1,  2,  2,  3,  3, 0,
                             4,  5, 5,  6,  6,  7,  7, 4,
                             8,  9, 9, 10, 10, 11, 11, 8,
                             1,  5,
                             6,  2,
                             4,  0,
                             3,  7,
                            12, 13
                        }
                    ));
                    sceneObj->addComponent (entity, RenderComponent (
                        static_cast <uint32_t> (instanceTransforms.size())
                    ));
                    sceneObj->addComponent (entity, WireTagComponent());
                }
                else
                    entityFamilyInfoPool[parentEntity].push_back (entity);
            }
        }
        {   /* Entity   [POINT_LIGHT_?] */
            Scene::Entity parentEntity;
            auto instanceTransforms = std::vector <glm::vec3> {
                {0.0f, 7.0f, 0.0f}
            };
            for (size_t i = 0; i < instanceTransforms.size(); i++) {
                auto entity = sceneObj->addEntity();

                sceneObj->addComponent (entity, MetaComponent (
                    "POINT_LIGHT_" + std::to_string (i),
                    TAG_TYPE_WIRE
                ));
                sceneObj->addComponent (entity, LightComponent (
                    LIGHT_TYPE_POINT,
                    {0.05f, 0.05f, 0.05f},
                    {0.80f, 0.80f, 0.80f},
                    {1.00f, 1.00f, 1.00f},
                    1.0f,
                    0.045f,
                    0.0075f,
                    R_180,
                    R_180,
                    0.01f,
                    100.0f
                ));
                sceneObj->addComponent (entity, TransformComponent (
                    instanceTransforms[i],
                    {0.0f, 0.0f, 0.0f},
                    {0.5f, 0.5f, 0.5f}
                ));
                sceneObj->addComponent (entity, ColorComponent (
                    {1.0f, 0.0f, 0.0f, 1.0f}
                ));

                if (i == 0) {   /* Parent entity */
                    parentEntity = entity;
                    entityFamilyInfoPool[parentEntity] = {};

                    sceneObj->addComponent (entity, MeshComponent (
                        {
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f, -0.5f,  0.5f}}, {0, 0, 0, 0}},      /* Vertex 0  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f, -0.5f,  0.5f}}, {0, 0, 0, 0}},      /* Vertex 1  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f,  0.5f,  0.5f}}, {0, 0, 0, 0}},      /* Vertex 2  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f,  0.5f,  0.5f}}, {0, 0, 0, 0}},      /* Vertex 3  */

                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.1f, -0.1f,  0.1f}}, {0, 0, 0, 0}},      /* Vertex 4  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.1f, -0.1f,  0.1f}}, {0, 0, 0, 0}},      /* Vertex 5  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.1f,  0.1f,  0.1f}}, {0, 0, 0, 0}},      /* Vertex 6  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.1f,  0.1f,  0.1f}}, {0, 0, 0, 0}},      /* Vertex 7  */

                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.1f, -0.1f, -0.1f}}, {0, 0, 0, 0}},      /* Vertex 8  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.1f, -0.1f, -0.1f}}, {0, 0, 0, 0}},      /* Vertex 9  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.1f,  0.1f, -0.1f}}, {0, 0, 0, 0}},      /* Vertex 10 */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.1f,  0.1f, -0.1f}}, {0, 0, 0, 0}},      /* Vertex 11 */

                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f, -0.5f, -0.5f}}, {0, 0, 0, 0}},      /* Vertex 12 */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f, -0.5f, -0.5f}}, {0, 0, 0, 0}},      /* Vertex 13 */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f,  0.5f, -0.5f}}, {0, 0, 0, 0}},      /* Vertex 14 */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f,  0.5f, -0.5f}}, {0, 0, 0, 0}}       /* Vertex 15 */
                        },
                        {
                             0,  1,  1,  2,  2,  3,  3,  0,
                             4,  5,  5,  6,  6,  7,  7,  4,
                             8,  9,  9, 10, 10, 11, 11,  8,
                            12, 13, 13, 14, 14, 15, 15, 12,
                             1, 13,
                            14,  2,
                             5,  9,
                            10,  6,
                            12,  0,
                             3, 15,
                             8,  4,
                             7, 11
                        }
                    ));
                    sceneObj->addComponent (entity, RenderComponent (
                        static_cast <uint32_t> (instanceTransforms.size())
                    ));
                    sceneObj->addComponent (entity, WireTagComponent());
                }
                else
                    entityFamilyInfoPool[parentEntity].push_back (entity);
            }
        }

        {   /* Entity   [DEBUG_CAMERA] */
            auto entity = sceneObj->addEntity();
            sceneObj->addComponent (entity, MetaComponent (
                "DEBUG_CAMERA",
                TAG_TYPE_NONE
            ));
            sceneObj->addComponent (entity, CameraComponent());
            sceneObj->addComponent (entity, TransformComponent (
                { 0.0f, 5.0f, 5.0f},
                {-R_45, 0.0f, 0.0f}
            ));
            /* No children */
            entityFamilyInfoPool[entity] = {};
        }
        {   /* Entity   [SCENE_CAMERA_?] */
            Scene::Entity parentEntity;
            auto instanceTransforms = std::vector <std::pair <glm::vec3, glm::vec3>> {
                {{36.0f, 6.0f, 36.0f}, {-R_90, 0.0f, 0.0f}}
            };
            for (size_t i = 0; i < instanceTransforms.size(); i++) {
                auto entity = sceneObj->addEntity();

                sceneObj->addComponent (entity, MetaComponent (
                    "SCENE_CAMERA_" + std::to_string (i),
                    TAG_TYPE_WIRE
                ));
                sceneObj->addComponent (entity, CameraComponent (
                    PROJECTION_TYPE_PERSPECTIVE,
                    false,
                    glm::radians (60.0f),
                    0.01f,
                    100.0f
                ));
                sceneObj->addComponent (entity, TransformComponent (
                    instanceTransforms[i].first,
                    instanceTransforms[i].second,
                    {0.5f, 0.5f, 0.5f}
                ));
                sceneObj->addComponent (entity, ColorComponent (
                    {0.0f, 0.0f, 1.0f, 1.0f}
                ));

                if (i == 0) {   /* Parent entity */
                    parentEntity = entity;
                    entityFamilyInfoPool[parentEntity] = {};

                    sceneObj->addComponent (entity, MeshComponent (
                        {
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { -0.1f,  -0.1f,  0.0f}}, {0, 0, 0, 0}},    /* Vertex 0  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {  0.1f,  -0.1f,  0.0f}}, {0, 0, 0, 0}},    /* Vertex 1  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {  0.1f,   0.1f,  0.0f}}, {0, 0, 0, 0}},    /* Vertex 2  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { -0.1f,   0.1f,  0.0f}}, {0, 0, 0, 0}},    /* Vertex 3  */

                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { -0.5f,  -0.5f, -1.0f}}, {0, 0, 0, 0}},    /* Vertex 4  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {  0.5f,  -0.5f, -1.0f}}, {0, 0, 0, 0}},    /* Vertex 5  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {  0.5f,   0.5f, -1.0f}}, {0, 0, 0, 0}},    /* Vertex 6  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { -0.5f,   0.5f, -1.0f}}, {0, 0, 0, 0}},    /* Vertex 7  */

                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.25f, 0.600f, -1.0f}}, {0, 0, 0, 0}},    /* Vertex 8  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.25f, 0.600f, -1.0f}}, {0, 0, 0, 0}},    /* Vertex 9  */
                            {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.00f, 1.033f, -1.0f}}, {0, 0, 0, 0}}     /* Vertex 10 */
                        },
                        {
                            0, 1, 1,  2,  2, 3, 3, 0,
                            4, 5, 5,  6,  6, 7, 7, 4,
                            8, 9, 9, 10, 10, 8,
                            1, 5,
                            6, 2,
                            4, 0,
                            3, 7
                        }
                    ));
                    sceneObj->addComponent (entity, RenderComponent (
                        static_cast <uint32_t> (instanceTransforms.size())
                    ));
                    sceneObj->addComponent (entity, WireTagComponent());
                }
                else
                    entityFamilyInfoPool[parentEntity].push_back (entity);
            }
        }

        {   /* Entity   [WORLD_BORDER_?] */
            Scene::Entity parentEntity;
            auto instanceTransforms = std::vector <std::pair <glm::vec3, glm::vec3>> {
                {{-30.0f, 1.0f, -40.0f}, {R_90, 0.0f, 0.0f}},
                {{-20.0f, 1.0f, -40.0f}, {R_90, 0.0f, 0.0f}},
                {{-10.0f, 1.0f, -40.0f}, {R_90, 0.0f, 0.0f}},
                {{  0.0f, 1.0f, -40.0f}, {R_90, 0.0f, 0.0f}},
                {{ 10.0f, 1.0f, -40.0f}, {R_90, 0.0f, 0.0f}},
                {{ 20.0f, 1.0f, -40.0f}, {R_90, 0.0f, 0.0f}},
                {{ 30.0f, 1.0f, -40.0f}, {R_90, 0.0f, 0.0f}}
            };
            for (size_t i = 0; i < instanceTransforms.size(); i++) {
                auto entity = sceneObj->addEntity();

                sceneObj->addComponent (entity, MetaComponent (
                    "WORLD_BORDER_" + std::to_string (i),
                    TAG_TYPE_STD_ALPHA
                ));
                sceneObj->addComponent (entity, TransformComponent (
                    instanceTransforms[i].first,
                    instanceTransforms[i].second
                ));
                sceneObj->addComponent (entity, TextureIdxLUTComponent());

                if (i == 0) {   /* Parent entity */
                    parentEntity = entity;
                    entityFamilyInfoPool[parentEntity] = {};
                    srcTextureIdxLUTComponentEntities.push_back (parentEntity);

                    sceneObj->addComponent (entity, MeshComponent (
                        "Asset/Model/World_Border.obj",
                        "Asset/Model/"
                    ));
                    sceneObj->addComponent (entity, RenderComponent (
                        static_cast <uint32_t> (instanceTransforms.size())
                    ));
                    sceneObj->addComponent (entity, StdAlphaTagComponent());
                }
                else
                    entityFamilyInfoPool[parentEntity].push_back (entity);
            }
        }

        {   /* Run pre-renderer-config systems */
            auto meshLoadingObj              = sceneObj->getSystem <SYMeshLoading>();
            auto meshBatchingObj             = sceneObj->getSystem <SYMeshBatching>();
            auto stdMeshInstanceBatchingObj  = sceneObj->getSystem <SYStdMeshInstanceBatching>();
            auto wireMeshInstanceBatchingObj = sceneObj->getSystem <SYWireMeshInstanceBatching>();
            auto lightInstanceBatchingObj    = sceneObj->getSystem <SYLightInstanceBatching>();

            {   /* Mesh loading system */
                meshLoadingObj->update();
                /* Populate texture idx LUT component for child entities */
                for (auto const& srcEntity: srcTextureIdxLUTComponentEntities) {
                for (auto const& dstEntity: entityFamilyInfoPool[srcEntity])   {

                    auto srcTextureIdxLUTComponent = sceneObj->getComponent <TextureIdxLUTComponent> (srcEntity);
                    auto dstTextureIdxLUTComponent = sceneObj->getComponent <TextureIdxLUTComponent> (dstEntity);
                    srcTextureIdxLUTComponent->copyToTextureIdxLUT (
                        dstTextureIdxLUTComponent->m_LUT
                    );
                }
                }
            }
            {   /* Mesh batching system */
                meshBatchingObj->update();
                meshBatchingObj->generateReport();
            }
            {   /* Std mesh instance batching system */
                stdMeshInstanceBatchingObj->update();
                stdMeshInstanceBatchingObj->generateReport();
            }
            {   /* Wire mesh instance batching system */
                wireMeshInstanceBatchingObj->update();
                wireMeshInstanceBatchingObj->generateReport();
            }
            {   /* Light instance batching system */
                lightInstanceBatchingObj->update (shadowImageWidth / static_cast <float> (shadowImageHeight));
                lightInstanceBatchingObj->generateReport();
            }
        }
    }
}   // namespace SandBox