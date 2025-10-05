#include <json/single_include/nlohmann/json.hpp>
#include "../../Backend/Common.h"
#include "../../Backend/Scene/SNImpl.h"
#include "../System/Loading/SYMeshLoading.h"
#include "../System/Batching/SYMeshBatching.h"
#include "../System/Batching/SYStdMeshInstanceBatching.h"
#include "../System/Batching/SYWireMeshInstanceBatching.h"
#include "../System/Batching/SYLightInstanceBatching.h"
#include "../System/Controller/SYCameraController.h"
#include "../System/Gui/SYSceneView.h"
#include "../System/Gui/SYEntityCollectionView.h"
#include "../System/Gui/SYComponentEditorView.h"
#include "../System/Gui/SYConfigView.h"
#include "../System/Rendering/SYShadowRendering.h"
#include "../System/Rendering/SYShadowCubeRendering.h"
#include "../System/Rendering/SYGDefaultRendering.h"
#include "../System/Rendering/SYLightRendering.h"
#include "../System/Rendering/SYWireRendering.h"
#include "../System/Rendering/SYSkyBoxRendering.h"
#include "../System/Rendering/SYFDefaultRendering.h"
#include "../System/Rendering/SYDebugRendering.h"
#include "../System/Rendering/SYGuiRendering.h"
#include "../SBImpl.h"
#include "../../Backend/Scene/SNType.h"
#include "../SBComponentType.h"
#include "../SBRendererType.h"

namespace SandBox {
    void SBImpl::configScene (void) {
        auto& meta     = m_sandBoxInfo.meta;
        auto& resource = m_sandBoxInfo.resource;
        auto& sceneObj = resource.sceneObj;

        /* Read and parse scene data file */
        std::ifstream file ("SandBox/Config/SBSceneData.json");
        if (!file.is_open())
            throw std::runtime_error ("Failed to open scene data file");

        auto sceneData = nlohmann::json::parse (file, nullptr, true, true);
        file.close();

        {   /* Register components */
            sceneObj->registerComponent <MetaComponent>();
            sceneObj->registerComponent <MeshComponent>();
            sceneObj->registerComponent <LightComponent>();
            sceneObj->registerComponent <CameraComponent>();
            sceneObj->registerComponent <TransformComponent>();
            sceneObj->registerComponent <TextureIdxOffsetComponent>();
            sceneObj->registerComponent <ColorComponent>();
            sceneObj->registerComponent <RenderComponent>();
            sceneObj->registerComponent <StdNoAlphaTagComponent>();
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
            auto sceneViewObj                = sceneObj->registerSystem <SYSceneView>();
            auto entityCollectionViewObj     = sceneObj->registerSystem <SYEntityCollectionView>();
            auto componentEditorViewObj      = sceneObj->registerSystem <SYComponentEditorView>();
            auto configViewObj               = sceneObj->registerSystem <SYConfigView>();
            auto shadowRenderingObj          = sceneObj->registerSystem <SYShadowRendering>();
            auto shadowCubeRenderingObj      = sceneObj->registerSystem <SYShadowCubeRendering>();
            auto gDefaultRenderingObj        = sceneObj->registerSystem <SYGDefaultRendering>();
            auto lightRenderingObj           = sceneObj->registerSystem <SYLightRendering>();
            auto wireRenderingObj            = sceneObj->registerSystem <SYWireRendering>();
            auto skyBoxRenderingObj          = sceneObj->registerSystem <SYSkyBoxRendering>();
            auto fDefaultRenderingObj        = sceneObj->registerSystem <SYFDefaultRendering>();
            auto debugRenderingObj           = sceneObj->registerSystem <SYDebugRendering>();
            auto guiRenderingObj             = sceneObj->registerSystem <SYGuiRendering>();

            /* Set system signature
             *                          +---+---+---+---+---+---+---+---+---+---+---+---+
             *                          | M | M | L | C | T | T | C | R | S | S | W | S |
             *                          | E | E | I | A | R | E | O | E | T | T | I | K |
             *                          | T | S | G | M | A | X | L | N | D | D | R | Y |
             *                          | A | H | H | E | N | T | O | D |   |   | E |   |
             *                          |   |   | T | R | S | U | R | E | N | A |   | B |
             *                          |   |   |   | A | F | R |   | R | O | L | T | O |
             *                          |   |   |   |   | O | E |   |   |   | P | A | X |
             *                          |   |   |   |   | R |   |   |   | A | H | G |   |
             *                          |   |   |   |   | M | I |   |   | L | A |   | T |
             *                          |   |   |   |   |   | D |   |   | P |   |   | A |
             *                          |   |   |   |   |   | X |   |   | H | T |   | G |
             *                          |   |   |   |   |   |   |   |   | A | A |   |   |
             *                          |   |   |   |   |   | O |   |   |   | G |   |   |
             *                          |   |   |   |   |   | F |   |   | T |   |   |   |
             *                          |   |   |   |   |   | F |   |   | A |   |   |   |
             *                          |   |   |   |   |   | S |   |   | G |   |   |   |
             *                          |   |   |   |   |   | E |   |   |   |   |   |   |
             *                          |   |   |   |   |   | T |   |   |   |   |   |   |
             *  +-----------------------+---+---+---+---+---+---+---+---+---+---+---+---+
             *  |                       Pre-renderer-config systems                     |
             *  +-----------------------+---+---+---+---+---+---+---+---+---+---+---+---+
             *  | Mesh                  |   |[o]|   |   |   |   |   |   |   |   |   |   |
             *  | loading               |   |   |   |   |   |   |   |   |   |   |   |   |
             *  +-----------------------+---+---+---+---+---+---+---+---+---+---+---+---+
             *  | Mesh                  |[o]|[o]|   |   |   |   |   |[o]|   |   |   |   |
             *  | batching              |   |   |   |   |   |   |   |   |   |   |   |   |
             *  +-----------------------+---+---+---+---+---+---+---+---+---+---+---+---+
             *  | Std mesh instance     |[o]|   |   |   |[o]|[o]|   |   |   |   |   |   |
             *  | batching              |   |   |   |   |   |   |   |   |   |   |   |   |
             *  +-----------------------+---+---+---+---+---+---+---+---+---+---+---+---+
             *  | Wire mesh instance    |   |   |   |   |[o]|   |[o]|   |   |   |   |   |
             *  | batching              |   |   |   |   |   |   |   |   |   |   |   |   |
             *  +-----------------------+---+---+---+---+---+---+---+---+---+---+---+---+
             *  | Light instance        |   |   |[o]|   |[o]|   |   |   |   |   |   |   |
             *  | batching              |   |   |   |   |   |   |   |   |   |   |   |   |
             *  +-----------------------+---+---+---+---+---+---+---+---+---+---+---+---+
             *  |                       Post-renderer-config systems                    |
             *  +-----------------------+---+---+---+---+---+---+---+---+---+---+---+---+
             *  | Camera                |   |   |   |[o]|[o]|   |   |   |   |   |   |   |
             *  | controller            |   |   |   |   |   |   |   |   |   |   |   |   |
             *  +-----------------------+---+---+---+---+---+---+---+---+---+---+---+---+
             *  | Scene                 |                                               |
             *  | view                  |                       ~                       |
             *  +-----------------------+-----------------------------------------------+
             *  | Entity collection     |                                               |
             *  | view                  |                       ~                       |
             *  +-----------------------+-----------------------------------------------+
             *  | Component editor      |                                               |
             *  | view                  |                       ~                       |
             *  +-----------------------+-----------------------------------------------+
             *  | Config                |                                               |
             *  | view                  |                       ~                       |
             *  +-----------------------+---+---+---+---+---+---+---+---+---+---+---+---+
             *  | Shadow                |   |   |   |   |   |   |   |[o]|[o]|   |   |   |
             *  | rendering             |   |   |   |   |   |   |   |   |   |   |   |   |
             *  +-----------------------+---+---+---+---+---+---+---+---+---+---+---+---+
             *  | Shadow cube           |   |   |   |   |   |   |   |[o]|[o]|   |   |   |
             *  | rendering             |   |   |   |   |   |   |   |   |   |   |   |   |
             *  +-----------------------+---+---+---+---+---+---+---+---+---+---+---+---+
             *  | G default             |   |   |   |   |   |   |   |[o]|[o]|   |   |   |
             *  | rendering             |   |   |   |   |   |   |   |   |   |   |   |   |
             *  +-----------------------+---+---+---+---+---+---+---+---+---+---+---+---+
             *  | Light                 |                                               |
             *  | rendering             |                       ~                       |
             *  +-----------------------+---+---+---+---+---+---+---+---+---+---+---+---+
             *  | Wire                  |   |   |   |   |   |   |   |[o]|   |   |[o]|   |
             *  | rendering             |   |   |   |   |   |   |   |   |   |   |   |   |
             *  +-----------------------+---+---+---+---+---+---+---+---+---+---+---+---+
             *  | Sky box               |   |   |   |   |   |   |   |[o]|   |   |   |[o]|
             *  | rendering             |   |   |   |   |   |   |   |   |   |   |   |   |
             *  +-----------------------+---+---+---+---+---+---+---+---+---+---+---+---+
             *  | F default             |   |   |   |   |   |   |   |[o]|   |[o]|   |   |
             *  | rendering             |   |   |   |   |   |   |   |   |   |   |   |   |
             *  +-----------------------+---+---+---+---+---+---+---+---+---+---+---+---+
             *  | Debug                 |                                               |
             *  | rendering             |                       ~                       |
             *  +-----------------------+-----------------------------------------------+
             *  | Gui                   |                                               |
             *  | rendering             |                       ~                       |
             *  +-----------------------+-----------------------------------------------+
            */
            {   /* Mesh loading system */
                meshLoadingObj->initMeshLoadingInfo (sceneObj, resource.stdTexturePoolObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <MeshComponent>());

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
                systemSignature.set (sceneObj->getComponentType <TextureIdxOffsetComponent>());

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
            {   /* Scene view system */
                static_cast <void> (sceneViewObj);
            }
            {   /* Entity collection view system */
                static_cast <void> (entityCollectionViewObj);
            }
            {   /* Component editor view system */
                static_cast <void> (componentEditorViewObj);
            }
            {   /* Config view system */
                static_cast <void> (configViewObj);
            }
            {   /* Shadow rendering system */
                static_cast <void> (shadowRenderingObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <RenderComponent>());
                systemSignature.set (sceneObj->getComponentType <StdNoAlphaTagComponent>());

                sceneObj->setSystemSignature <SYShadowRendering> (systemSignature);
            }
            {   /* Shadow cube rendering system */
                static_cast <void> (shadowCubeRenderingObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <RenderComponent>());
                systemSignature.set (sceneObj->getComponentType <StdNoAlphaTagComponent>());

                sceneObj->setSystemSignature <SYShadowCubeRendering> (systemSignature);
            }
            {   /* G default rendering system */
                static_cast <void> (gDefaultRenderingObj);

                Scene::Signature systemSignature;
                systemSignature.set (sceneObj->getComponentType <RenderComponent>());
                systemSignature.set (sceneObj->getComponentType <StdNoAlphaTagComponent>());

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
            {   /* Gui rendering system */
                static_cast <void> (guiRenderingObj);
            }
        }

        /* Add entities and components */
        Scene::Entity parentEntity;
        for (auto const& entityData: sceneData["entities"]) {
            auto entity = sceneObj->addEntity();

            {   /* Meta component */
                if (!entityData["meta"].is_null())
                    sceneObj->addComponent (entity, MetaComponent (
                        entityData["meta"]["id"],
                        getTagTypeEnum (entityData["meta"]["tagType"])
                    ));
            }
            {   /* Mesh component */
                if (!entityData["mesh"].is_null()) {

                    if (!entityData["mesh"]["modelFilePath"].is_null() &&
                        !entityData["mesh"]["mtlFileDirPath"].is_null())
                        sceneObj->addComponent (entity, MeshComponent (
                            entityData["mesh"]["modelFilePath"],
                            entityData["mesh"]["mtlFileDirPath"]
                        ));

                    else if (!entityData["mesh"]["vertices"].is_null() &&
                             !entityData["mesh"]["indices"].is_null()) {

                        auto vertices = std::vector <Vertex>    {};
                        auto indices  = std::vector <IndexType> {};
                        for (auto const& vertexData: entityData["mesh"]["vertices"]) {
                            Vertex vertex;

                            vertex.meta.uv                     = {0.0f, 0.0f};
                            vertex.meta.normal                 = {0.0f, 0.0f, 0.0f};
                            vertex.meta.position               = {vertexData[0], vertexData[1], vertexData[2]};

                            vertex.material.diffuseTextureIdx  = 0;
                            vertex.material.specularTextureIdx = 0;
                            vertex.material.emissionTextureIdx = 0;
                            vertex.material.shininess          = 0;

                            vertices.push_back (vertex);
                        }
                        for (auto const& indexData: entityData["mesh"]["indices"])
                            indices.push_back (indexData);

                        sceneObj->addComponent (entity, MeshComponent (
                            vertices,
                            indices
                        ));
                    }
                }
            }
            {   /* Light component */
                if (!entityData["light"].is_null())
                    sceneObj->addComponent (entity, LightComponent (
                        getLightTypeEnum (entityData["light"]["lightType"]),
                        {
                            entityData["light"]["ambient"][0],
                            entityData["light"]["ambient"][1],
                            entityData["light"]["ambient"][2]
                        },
                        {
                            entityData["light"]["diffuse"][0],
                            entityData["light"]["diffuse"][1],
                            entityData["light"]["diffuse"][2]
                        },
                        {
                            entityData["light"]["specular"][0],
                            entityData["light"]["specular"][1],
                            entityData["light"]["specular"][2]
                        },
                        entityData["light"]["constant"],
                        entityData["light"]["linear"],
                        entityData["light"]["quadratic"],
                        entityData["light"]["innerRadius"],
                        entityData["light"]["outerRadius"],
                        entityData["light"]["nearPlane"],
                        entityData["light"]["farPlane"]
                    ));
            }
            {   /* Camera component */
                if (!entityData["camera"].is_null())
                    sceneObj->addComponent (entity, CameraComponent (
                        getProjectionTypeEnum (entityData["camera"]["projectionType"]),
                        entityData["camera"]["fov"],
                        entityData["camera"]["nearPlane"],
                        entityData["camera"]["farPlane"]
                    ));
            }
            {   /* Transform component */
                if (!entityData["transform"].is_null())
                    sceneObj->addComponent (entity, TransformComponent (
                        {
                            entityData["transform"]["position"][0],
                            entityData["transform"]["position"][1],
                            entityData["transform"]["position"][2]
                        },
                        {
                            entityData["transform"]["rotation"][0],
                            entityData["transform"]["rotation"][1],
                            entityData["transform"]["rotation"][2]
                        },
                        {
                            entityData["transform"]["scale"][0],
                            entityData["transform"]["scale"][1],
                            entityData["transform"]["scale"][2]
                        }
                    ));
            }
            {   /* Texture idx offset component */
                if (!entityData["textureIdxOffset"].is_null())
                    sceneObj->addComponent (entity, TextureIdxOffsetComponent (
                        entityData["textureIdxOffset"]["diffuseTextureIdxOffset"],
                        entityData["textureIdxOffset"]["specularTextureIdxOffset"],
                        entityData["textureIdxOffset"]["emissionTextureIdxOffset"]
                    ));
            }
            {   /* Color component */
                if (!entityData["color"].is_null())
                    sceneObj->addComponent (entity, ColorComponent (
                        {
                            entityData["color"][0],
                            entityData["color"][1],
                            entityData["color"][2],
                            entityData["color"][3]
                        }
                    ));
            }
            {   /* Render component */
                if (!entityData["render"].is_null())
                    sceneObj->addComponent (entity, RenderComponent (
                        entityData["render"]["instancesCount"]
                    ));
            }
            {   /* Std no alpha tag component */
                if (!entityData["stdNoAlphaTag"].is_null() && entityData["stdNoAlphaTag"])
                    sceneObj->addComponent (entity, StdNoAlphaTagComponent());
            }
            {   /* Std alpha tag component */
                if (!entityData["stdAlphaTag"].is_null() && entityData["stdAlphaTag"])
                    sceneObj->addComponent (entity, StdAlphaTagComponent());
            }
            {   /* Wire tag component */
                if (!entityData["wireTag"].is_null() && entityData["wireTag"])
                    sceneObj->addComponent (entity, WireTagComponent());
            }
            {   /* Sky box tag component */
                if (!entityData["skyBoxTag"].is_null() && entityData["skyBoxTag"])
                    sceneObj->addComponent (entity, SkyBoxTagComponent());
            }

            /* Parse helper */
            if (!entityData["parseHelper"].is_null()) {
                /* Save sky box entity */
                if (entityData["parseHelper"]["skyBoxEntity"])
                    meta.skyBoxEntity = entity;

                /* Save active camera entity */
                if (entityData["parseHelper"]["activeCameraEntity"])
                    meta.activeCameraEntity = entity;

                /* Save entity parent-children relation (ordererd as shown below)
                 *  +-------+-------+-------+-------+-------+-------+-------+-------+       +-------+
                 *  |  A,0  |  A,1  |  A,2  |  A,3  |  B,0  |  C,0  |  C,1  |  C,2  | ..... |  C,n  |
                 *  +-------+-------+-------+-------+-------+-------+-------+-------+       +-------+
                 *      |       |               |       |       |       |                       |
                 *      v       +---------------+       v       v       +-----------------------+
                 *    Parent        Children          Parent  Parent            Children
                */
                if (entityData["parseHelper"]["parentEntity"]) {
                    parentEntity = entity;
                    meta.parentEntityToChildrenMap[parentEntity] = {};
                }
                else
                    meta.parentEntityToChildrenMap[parentEntity].push_back (entity);
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
                lightInstanceBatchingObj->update (meta.shadowImageWidth / static_cast <float> (meta.shadowImageHeight));
                lightInstanceBatchingObj->generateReport();
            }
        }
    }
}   // namespace SandBox