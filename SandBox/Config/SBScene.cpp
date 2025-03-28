#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <utility>
#include "../../Backend/Scene/SNImpl.h"
#include "../System/SYMeshLoading.h"
#include "../System/SYMeshBatching.h"
#include "../System/SYMeshInstanceBatching.h"
#include "../System/SYLightInstanceBatching.h"
#include "../System/SYCameraController.h"
#include "../System/SYDefaultRendering.h"
#include "../SBImpl.h"
#include "../../Backend/Scene/SNType.h"
#include "../SBComponentType.h"

namespace SandBox {
    void SBImpl::configScene (void) {
        auto& sceneObj = m_sandBoxInfo.resource.sceneObj;
        {   /* Register components */
            sceneObj->registerComponent <IdComponent>();
            sceneObj->registerComponent <MeshComponent>();
            sceneObj->registerComponent <LightComponent>();
            sceneObj->registerComponent <CameraComponent>();
            sceneObj->registerComponent <TransformComponent>();
            sceneObj->registerComponent <TextureIdxLUTComponent>();
            sceneObj->registerComponent <RenderComponent>();
        }
        {   /* Register systems */
            auto meshLoadingObj           = sceneObj->registerSystem <SYMeshLoading>();
            auto meshBatchingObj          = sceneObj->registerSystem <SYMeshBatching>();
            auto meshInstanceBatchingObj  = sceneObj->registerSystem <SYMeshInstanceBatching>();
            auto lightInstanceBatchingObj = sceneObj->registerSystem <SYLightInstanceBatching>();
            auto cameraControllerObj      = sceneObj->registerSystem <SYCameraController>();
            auto defaultRenderingObj      = sceneObj->registerSystem <SYDefaultRendering>();
            /* Set system signature */
            {   /* Mesh loading system */
                meshLoadingObj->initMeshLoadingInfo (sceneObj);

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
            {   /* Mesh instance batching system */
                meshInstanceBatchingObj->initMeshInstanceBatchingInfo (sceneObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <TransformComponent>());
                systemSignature.set (sceneObj->getComponentType <TextureIdxLUTComponent>());

                sceneObj->setSystemSignature <SYMeshInstanceBatching> (systemSignature);
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
            {   /* Default rendering system */
                static_cast <void> (defaultRenderingObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <RenderComponent>());

                sceneObj->setSystemSignature <SYDefaultRendering> (systemSignature);
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
        {   /* Entity   [DEBUG_MESH_?] */
            uint32_t instancesCount = 8;
            auto instanceTransforms = std::vector <std::pair <glm::vec3, glm::vec3>> {
                {{-2.0f,  0.5f, -2.0f }, { 0.00f, 0.00f, 0.00f }},      /* Parent transform */
                {{ 0.0f,  0.5f, -2.0f }, { 0.00f, 0.00f, 1.57f }},      /* Roll             */
                {{ 2.0f,  0.5f, -2.0f }, { 0.00f, 1.57f, 0.00f }},      /* Yaw              */
                {{ 2.0f,  0.5f,  0.0f }, { 0.00f, 1.57f, 1.57f }},      /* Roll->Yaw        */
                {{ 0.0f,  0.5f,  0.0f }, { 1.57f, 0.00f, 0.00f }},      /* Pitch            */
                {{-2.0f,  0.5f,  0.0f }, { 1.57f, 0.00f, 1.57f }},      /* Roll->Pitch      */
                {{-2.0f,  0.5f,  2.0f }, { 1.57f, 1.57f, 0.00f }},      /* Pitch->Yaw       */
                {{ 0.0f,  0.5f,  2.0f }, { 1.57f, 1.57f, 1.57f }}       /* Roll->Pitch->Yaw */
            };
            for (uint32_t i = 0; i < instancesCount; i++) {
                auto entity = sceneObj->addEntity();

                sceneObj->addComponent (entity, IdComponent (
                    "DEBUG_MESH_" + std::to_string (i)
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
            auto entity = sceneObj->addEntity();
            sceneObj->addComponent (entity, IdComponent (
                "DEBUG_LIGHT"
            ));
            sceneObj->addComponent (entity, LightComponent());
            sceneObj->addComponent (entity, TransformComponent (
                { 0.00f, 5.00f, -5.00f},
                {-0.78f, 3.14f,  0.00f}
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

        {   /* Run pre-renderer-config systems */
            auto meshLoadingObj           = sceneObj->getSystem <SYMeshLoading>();
            auto meshBatchingObj          = sceneObj->getSystem <SYMeshBatching>();
            auto meshInstanceBatchingObj  = sceneObj->getSystem <SYMeshInstanceBatching>();
            auto lightInstanceBatchingObj = sceneObj->getSystem <SYLightInstanceBatching>();

            {   /* Mesh loading system */
                meshLoadingObj->update();
                auto texturePoolObj = meshLoadingObj->getTexturePoolObj();
                /* Populate texture idx LUT component for child instances */
                {   /* Entity 1..7 */
                    for (Scene::Entity i = 1; i < 8; i++) {
                        auto srcTextureIdxLUTComponent = sceneObj->getComponent <TextureIdxLUTComponent> (0);
                        auto dstTextureIdxLUTComponent = sceneObj->getComponent <TextureIdxLUTComponent> (i);
                        srcTextureIdxLUTComponent->copyToTextureIdxLUT (
                            dstTextureIdxLUTComponent->m_textureIdxLUT
                        );
                    }
                }
                texturePoolObj->generateReport();
            }
            {   /* Mesh batching system */
                meshBatchingObj->update();
                meshBatchingObj->generateReport();
            }
            {   /* Mesh instance batching system */
                meshInstanceBatchingObj->update();
                meshInstanceBatchingObj->generateReport();
            }
            {   /* Light instance batching system */
                lightInstanceBatchingObj->update();
                lightInstanceBatchingObj->generateReport();
            }
        }
        {   /* Remove render component */
            sceneObj->removeComponent <RenderComponent> (0);
        }
    }
}   // namespace SandBox