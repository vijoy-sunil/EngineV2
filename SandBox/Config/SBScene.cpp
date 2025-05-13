#include "../../Backend/Common.h"
#include "../../Backend/Scene/SNImpl.h"
#include "../System/SYMeshLoading.h"
#include "../System/SYMeshBatching.h"
#include "../System/SYDefaultMeshInstanceBatching.h"
#include "../System/SYWireMeshInstanceBatching.h"
#include "../System/SYDefaultLightInstanceBatching.h"
#include "../System/SYCameraController.h"
#include "../System/SYDefaultRendering.h"
#include "../System/SYWireRendering.h"
#include "../System/SYSkyBoxRendering.h"
#include "../SBImpl.h"
#include "../../Backend/Scene/SNType.h"
#include "../SBComponentType.h"

namespace SandBox {
    void SBImpl::configScene (void) {
        auto& skyBoxEntity          = m_sandBoxInfo.meta.skyBoxEntity;
        auto& sceneObj              = m_sandBoxInfo.resource.sceneObj;
        auto& defaultTexturePoolObj = m_sandBoxInfo.resource.defaultTexturePoolObj;
        {   /* Register components */
            sceneObj->registerComponent <IdComponent>();
            sceneObj->registerComponent <MeshComponent>();
            sceneObj->registerComponent <LightComponent>();
            sceneObj->registerComponent <CameraComponent>();
            sceneObj->registerComponent <TransformComponent>();
            sceneObj->registerComponent <TextureIdxLUTComponent>();
            sceneObj->registerComponent <ColorComponent>();
            sceneObj->registerComponent <RenderComponent>();
        }
        {   /* Register systems */
            auto meshLoadingObj                  = sceneObj->registerSystem <SYMeshLoading>();
            auto meshBatchingObj                 = sceneObj->registerSystem <SYMeshBatching>();
            auto defaultMeshInstanceBatchingObj  = sceneObj->registerSystem <SYDefaultMeshInstanceBatching>();
            auto wireMeshInstanceBatchingObj     = sceneObj->registerSystem <SYWireMeshInstanceBatching>();
            auto defaultLightInstanceBatchingObj = sceneObj->registerSystem <SYDefaultLightInstanceBatching>();
            auto cameraControllerObj             = sceneObj->registerSystem <SYCameraController>();
            auto defaultRenderingObj             = sceneObj->registerSystem <SYDefaultRendering>();
            auto wireRenderingObj                = sceneObj->registerSystem <SYWireRendering>();
            auto skyBoxRenderingObj              = sceneObj->registerSystem <SYSkyBoxRendering>();
            /* Set system signature */
            {   /* Mesh loading system */
                meshLoadingObj->initMeshLoadingInfo (sceneObj, defaultTexturePoolObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <MeshComponent>());
                systemSignature.set (sceneObj->getComponentType <TextureIdxLUTComponent>());

                sceneObj->setSystemSignature <SYMeshLoading> (systemSignature);
            }
            {   /* Mesh batching system */
                meshBatchingObj->initMeshBatchingInfo (sceneObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <MeshComponent>());
                systemSignature.set (sceneObj->getComponentType <RenderComponent>());

                sceneObj->setSystemSignature <SYMeshBatching> (systemSignature);
            }
            {   /* Default mesh instance batching system */
                defaultMeshInstanceBatchingObj->initDefaultMeshInstanceBatchingInfo (sceneObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <TransformComponent>());
                systemSignature.set (sceneObj->getComponentType <TextureIdxLUTComponent>());

                sceneObj->setSystemSignature <SYDefaultMeshInstanceBatching> (systemSignature);
            }
            {   /* Wire mesh instance batching system */
                wireMeshInstanceBatchingObj->initWireMeshInstanceBatchingInfo (sceneObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <TransformComponent>());
                systemSignature.set (sceneObj->getComponentType <ColorComponent>());

                sceneObj->setSystemSignature <SYWireMeshInstanceBatching> (systemSignature);
            }
            {   /* Default light instance batching system */
                defaultLightInstanceBatchingObj->initDefaultLightInstanceBatchingInfo (sceneObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <LightComponent>());
                systemSignature.set (sceneObj->getComponentType <TransformComponent>());

                sceneObj->setSystemSignature <SYDefaultLightInstanceBatching> (systemSignature);
            }
            {   /* Camera controller system */
                /* Init method called post-renderer-config */
                static_cast <void> (cameraControllerObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <CameraComponent>());
                systemSignature.set (sceneObj->getComponentType <TransformComponent>());

                sceneObj->setSystemSignature <SYCameraController> (systemSignature);
            }
            {   /* Default rendering system */
                static_cast <void> (defaultRenderingObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <RenderComponent>());

                sceneObj->setSystemSignature <SYDefaultRendering> (systemSignature);
            }
            {   /* Wire rendering system */
                static_cast <void> (wireRenderingObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <RenderComponent>());

                sceneObj->setSystemSignature <SYWireRendering> (systemSignature);
            }
            {   /* Sky box rendering system */
                static_cast <void> (skyBoxRenderingObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <RenderComponent>());

                sceneObj->setSystemSignature <SYSkyBoxRendering> (systemSignature);
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
            uint32_t instancesCount = 8;
            auto instanceTransforms = std::vector <std::pair <glm::vec3, glm::vec3>> {
                {{-2.0f,  0.5f, -2.0f }, { 0.00f, 0.00f, 0.00f }},      /* Parent transform */
                {{ 0.0f,  0.5f, -2.0f }, { 0.00f, 1.57f, 0.00f }},      /* Yaw              */
                {{ 2.0f,  0.5f, -2.0f }, { 1.57f, 0.00f, 0.00f }},      /* Pitch            */
                {{ 2.0f,  0.5f,  0.0f }, { 0.00f, 0.00f, 1.57f }},      /* Roll             */
                {{ 0.0f,  0.5f,  0.0f }, { 1.57f, 1.57f, 0.00f }},      /* Yaw->Pitch       */
                {{-2.0f,  0.5f,  0.0f }, { 0.00f, 1.57f, 1.57f }},      /* Yaw->Roll        */
                {{-2.0f,  0.5f,  2.0f }, { 1.57f, 0.00f, 1.57f }},      /* Pitch->Roll      */
                {{ 0.0f,  0.5f,  2.0f }, { 1.57f, 1.57f, 1.57f }}       /* Yaw->Pitch->Roll */
            };
            for (uint32_t i = 0; i < instancesCount; i++) {
                auto entity = sceneObj->addEntity();

                sceneObj->addComponent (entity, IdComponent (
                    "DEBUG_CUBE_" + std::to_string (i)
                ));
                sceneObj->addComponent (entity, TransformComponent (
                    instanceTransforms[i].first,
                    instanceTransforms[i].second
                ));
                sceneObj->addComponent (entity, TextureIdxLUTComponent());
            }
            /* Append to parent entity components */
            sceneObj->addComponent (0, MeshComponent());
            sceneObj->addComponent (0, RenderComponent (
                TAG_TYPE_DEFAULT,
                instancesCount
            ));
        }
        {   /* Entity   [DEBUG_SPHERE] */
            auto entity = sceneObj->addEntity();
            sceneObj->addComponent (entity, IdComponent (
                "DEBUG_SPHERE"
            ));
            sceneObj->addComponent (entity, MeshComponent (
                "Asset/Model/Debug_Sphere.obj",
                "Asset/Model/"
            ));
            sceneObj->addComponent (entity, TransformComponent (
                {0.0f, 2.0f, 0.0f},
                {0.0f, 0.0f, 0.0f}
            ));
            sceneObj->addComponent (entity, TextureIdxLUTComponent());
            sceneObj->addComponent (entity, RenderComponent (
                TAG_TYPE_DEFAULT,
                1
            ));
        }
        {   /* Entity   [SKY_BOX] */
            auto entity = sceneObj->addEntity();
            sceneObj->addComponent (entity, IdComponent (
                "SKY_BOX"
            ));
            sceneObj->addComponent (entity, MeshComponent (
                "Asset/Model/Sky_Box.obj",
                "Asset/Model/"
            ));
            sceneObj->addComponent (entity, TransformComponent());
            sceneObj->addComponent (entity, TextureIdxLUTComponent());
            sceneObj->addComponent (entity, RenderComponent (
                TAG_TYPE_SKY_BOX,
                1
            ));
            /* Save sky box entity */
            skyBoxEntity = entity;
        }
        {   /* Entity   [GROUND_PLANE] */
            auto entity = sceneObj->addEntity();
            sceneObj->addComponent (entity, IdComponent (
                "GROUND_PLANE"
            ));
            sceneObj->addComponent (entity, MeshComponent (
                "Asset/Model/Ground_Plane.obj",
                "Asset/Model/"
            ));
            sceneObj->addComponent (entity, TransformComponent());
            sceneObj->addComponent (entity, TextureIdxLUTComponent());
            sceneObj->addComponent (entity, RenderComponent (
                TAG_TYPE_DEFAULT,
                1
            ));
        }
        {   /* Entity   [DEBUG_LIGHT] */
            /* Directional light */
            auto entity = sceneObj->addEntity();
            sceneObj->addComponent (entity, IdComponent (
                "DEBUG_LIGHT"
            ));
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
                    8,  9,  9,  10, 10, 11, 11, 8,
                    12, 13, 13, 14, 14, 15, 15, 12,
                    16, 17
                }
            ));
            sceneObj->addComponent (entity, LightComponent());
            sceneObj->addComponent (entity, TransformComponent (
                { 0.00f, 5.00f, -5.00f},
                {-0.78f, 3.14f,  0.00f},
                { 0.50f, 0.50f,  0.50f}
            ));
            sceneObj->addComponent (entity, ColorComponent (
                {1.0f, 0.0f, 0.0f, 1.0f}
            ));
            sceneObj->addComponent (entity, RenderComponent (
                TAG_TYPE_WIRE,
                1
            ));
        }
        {   /* Entity   [POINT_LIGHT_?] */
            uint32_t instancesCount = 2;
            auto instanceTransforms = std::vector <std::pair <glm::vec3, glm::vec3>> {
                {{ 2.0f, 2.0f, -2.0f }, { 0.0f, 0.0f, 0.0f }},
                {{-2.0f, 2.0f,  2.0f }, { 0.0f, 0.0f, 0.0f }}
            };
            for (uint32_t i = 0; i < instancesCount; i++) {
                auto entity = sceneObj->addEntity();

                sceneObj->addComponent (entity, IdComponent (
                    "POINT_LIGHT_" + std::to_string (i)
                ));
                sceneObj->addComponent (entity, LightComponent (
                    LIGHT_TYPE_POINT,
                    {0.05f, 0.05f, 0.05f},
                    {0.80f, 0.80f, 0.80f},
                    {1.00f, 1.00f, 1.00f},
                    1.0f,
                    0.045f,
                    0.0075f,
                    glm::radians (180.0f),
                    glm::radians (180.0f)
                ));
                sceneObj->addComponent (entity, TransformComponent (
                    instanceTransforms[i].first,
                    instanceTransforms[i].second,
                    {0.5f, 0.5f, 0.5f}
                ));
                sceneObj->addComponent (entity, ColorComponent (
                    {1.0f, 0.0f, 0.0f, 1.0f}
                ));
            }
            /* Append to parent entity components */
            sceneObj->addComponent (12, MeshComponent (
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
                    8,  9,  9,  10, 10, 11, 11, 8,
                    12, 13, 13, 14, 14, 15, 15, 12,
                    1,  13,
                    5,  9,
                    6,  10,
                    2,  14,
                    12, 0,
                    8,  4,
                    11, 7,
                    15, 3
                }
            ));
            sceneObj->addComponent (12, RenderComponent (
                TAG_TYPE_WIRE,
                instancesCount
            ));
        }
        {   /* Entity   [SPOT_LIGHT_?] */
            uint32_t instancesCount = 2;
            auto instanceTransforms = std::vector <std::pair <glm::vec3, glm::vec3>> {
                {{-2.0f, 2.0f, -2.0f }, {-1.57f, 0.0f, 0.0f }},
                {{ 0.0f, 2.0f,  2.0f }, {-1.57f, 0.0f, 0.0f }}
            };
            for (uint32_t i = 0; i < instancesCount; i++) {
                auto entity = sceneObj->addEntity();

                sceneObj->addComponent (entity, IdComponent (
                    "SPOT_LIGHT_" + std::to_string (i)
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
                    glm::radians (30.0f)
                ));
                sceneObj->addComponent (entity, TransformComponent (
                    instanceTransforms[i].first,
                    instanceTransforms[i].second,
                    {0.5f, 0.5f, 0.5f}
                ));
                sceneObj->addComponent (entity, ColorComponent (
                    {1.0f, 0.0f, 0.0f, 1.0f}
                ));
            }
            /* Append to parent entity components */
            sceneObj->addComponent (14, MeshComponent (
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
                    0,  1,  1,  2,  2,  3,  3,  0,
                    4,  5,  5,  6,  6,  7,  7,  4,
                    8,  9,  9,  10, 10, 11, 11, 8,
                    1,  5,
                    2,  6,
                    4,  0,
                    7,  3,
                    12, 13
                }
            ));
            sceneObj->addComponent (14, RenderComponent (
                TAG_TYPE_WIRE,
                instancesCount
            ));
        }
        {   /* Entity   [DEBUG_CAMERA] */
            auto entity = sceneObj->addEntity();
            sceneObj->addComponent (entity, IdComponent (
                "DEBUG_CAMERA"
            ));
            sceneObj->addComponent (entity, CameraComponent());
            sceneObj->addComponent (entity, TransformComponent (
                { 0.00f, 5.00f, 5.00f},
                {-0.78f, 0.00f, 0.00f}
            ));
        }
        {   /* Entity   [SCENE_CAMERA] */
            auto entity = sceneObj->addEntity();
            sceneObj->addComponent (entity, IdComponent (
                "SCENE_CAMERA"
            ));
            sceneObj->addComponent (entity, MeshComponent (
                {
                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.1f, -0.1f,  0.0f}},    {0, 0, 0, 0}},   /* Vertex 0  */
                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.1f, -0.1f,  0.0f}},    {0, 0, 0, 0}},   /* Vertex 1  */
                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.1f,  0.1f,  0.0f}},    {0, 0, 0, 0}},   /* Vertex 2  */
                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.1f,  0.1f,  0.0f}},    {0, 0, 0, 0}},   /* Vertex 3  */

                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f, -0.5f, -1.0f}},    {0, 0, 0, 0}},   /* Vertex 4  */
                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f, -0.5f, -1.0f}},    {0, 0, 0, 0}},   /* Vertex 5  */
                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.5f,  0.5f, -1.0f}},    {0, 0, 0, 0}},   /* Vertex 6  */
                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.5f,  0.5f, -1.0f}},    {0, 0, 0, 0}},   /* Vertex 7  */

                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {-0.25f,  0.600f, -1.0f}}, {0, 0, 0, 0}},   /* Vertex 8  */
                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.25f,  0.600f, -1.0f}}, {0, 0, 0, 0}},   /* Vertex 9  */
                    {{{0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, { 0.00f,  1.033f, -1.0f}}, {0, 0, 0, 0}}    /* Vertex 10 */
                },
                {
                    0,  1,  1,  2,  2,  3,  3,  0,
                    4,  5,  5,  6,  6,  7,  7,  4,
                    8,  9,  9,  10, 10, 8,
                    1,  5,
                    2,  6,
                    4,  0,
                    7,  3
                }
            ));
            sceneObj->addComponent (entity, CameraComponent (
                CAMERA_TYPE_SCENE,
                PROJECTION_TYPE_PERSPECTIVE,
                false,
                glm::radians (60.0f),
                0.1f,
                100.0f,
                5.0f
            ));
            sceneObj->addComponent (entity, TransformComponent (
                {1.0f, 1.0f, 1.0f},
                {0.0f, 0.0f, 0.0f},
                {0.5f, 0.5f, 0.5f}
            ));
            sceneObj->addComponent (entity, ColorComponent (
                {0.0f, 0.0f, 1.0f, 1.0f}
            ));
            sceneObj->addComponent (entity, RenderComponent (
                TAG_TYPE_WIRE,
                1
            ));
        }
        {   /* Entity   [DEBUG_PLANE] */
            auto entity = sceneObj->addEntity();
            sceneObj->addComponent (entity, IdComponent (
                "DEBUG_PLANE"
            ));
            sceneObj->addComponent (entity, MeshComponent (
                "Asset/Model/Debug_Plane.obj",
                "Asset/Model/"
            ));
            sceneObj->addComponent (entity, TransformComponent (
                { 0.00f, 1.00f, 4.00f},
                {-1.57f, 0.00f, 0.00f}
            ));
            sceneObj->addComponent (entity, TextureIdxLUTComponent());
            sceneObj->addComponent (entity, RenderComponent (
                TAG_TYPE_DEFAULT,
                1
            ));
        }

        {   /* Run pre-renderer-config systems */
            auto meshLoadingObj                  = sceneObj->getSystem <SYMeshLoading>();
            auto meshBatchingObj                 = sceneObj->getSystem <SYMeshBatching>();
            auto defaultMeshInstanceBatchingObj  = sceneObj->getSystem <SYDefaultMeshInstanceBatching>();
            auto wireMeshInstanceBatchingObj     = sceneObj->getSystem <SYWireMeshInstanceBatching>();
            auto defaultLightInstanceBatchingObj = sceneObj->getSystem <SYDefaultLightInstanceBatching>();

            {   /* Mesh loading system */
                meshLoadingObj->update();
                /* Populate texture idx LUT component for child instances */
                {   /* Entity 1..7 */
                    for (Scene::Entity i = 1; i < 8; i++) {
                        auto srcTextureIdxLUTComponent = sceneObj->getComponent <TextureIdxLUTComponent> (0);
                        auto dstTextureIdxLUTComponent = sceneObj->getComponent <TextureIdxLUTComponent> (i);
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
            {   /* Default mesh instance batching system */
                {   /* Remove texture idx LUT component */
                    sceneObj->removeComponent <TextureIdxLUTComponent> (9);     /* SKY_BOX */
                }
                defaultMeshInstanceBatchingObj->update();
                defaultMeshInstanceBatchingObj->generateReport();
            }
            {   /* Wire mesh instance batching system */
                wireMeshInstanceBatchingObj->update();
                wireMeshInstanceBatchingObj->generateReport();
            }
            {   /* Default light instance batching system */
                defaultLightInstanceBatchingObj->update();
                defaultLightInstanceBatchingObj->generateReport();
            }
        }
        {   /* Remove render component */
            sceneObj->removeComponent <RenderComponent> (0);    /* DEBUG_CUBE_0 */
            sceneObj->removeComponent <RenderComponent> (8);    /* DEBUG_SPHERE */
        }
    }
}   // namespace SandBox