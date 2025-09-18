#pragma once
#include "../Backend/Common.h"
#include "../Backend/Scene/SNImpl.h"
#include "../Backend/Collection/CNImpl.h"
#include "../Backend/Renderer/VKWindow.h"
#include "../Backend/Renderer/VKLogDevice.h"
#include "../Backend/Renderer/VKRenderer.h"
#include "../Backend/Renderer/VKGui.h"
#include "System/Batching/SYStdMeshInstanceBatching.h"
#include "System/Batching/SYWireMeshInstanceBatching.h"
#include "System/Batching/SYLightInstanceBatching.h"
#include "System/Controller/SYCameraController.h"
#include "System/Gui/SYSceneView.h"
#include "System/Gui/SYEntityCollectionView.h"
#include "System/Gui/SYComponentEditorView.h"
#include "System/Gui/SYConfigView.h"
#include "System/Rendering/SYShadowRendering.h"
#include "System/Rendering/SYShadowCubeRendering.h"
#include "System/Rendering/SYGDefaultRendering.h"
#include "System/Rendering/SYLightRendering.h"
#include "System/Rendering/SYWireRendering.h"
#include "System/Rendering/SYSkyBoxRendering.h"
#include "System/Rendering/SYFDefaultRendering.h"
#include "System/Rendering/SYDebugRendering.h"
#include "System/Rendering/SYGuiRendering.h"
#include "SBTexturePool.h"
#include "../Backend/Scene/SNType.h"
#include "System/SYConfig.h"
#include "SBComponentType.h"
#include "SBRendererType.h"

#define PROFILE_CAPTURE                 std::chrono::high_resolution_clock::now()
#define PROFILE_COMPUTE(begin, end)     std::chrono::duration <float, std::chrono::seconds::period> \
                                        (end - begin).count()

namespace SandBox {
    class SBImpl {
        private:
            struct SandBoxInfo {
                struct Meta {
                    uint32_t shadowImageWidth;
                    uint32_t shadowImageHeight;

                    Scene::Entity skyBoxEntity;
                    Scene::Entity activeCameraEntity;
                    std::map <Scene::Entity, std::vector <Scene::Entity>> parentEntityToChildrenMap;
                } meta;

                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Collection::CNImpl* collectionObj;
                    SBTexturePool* stdTexturePoolObj;
                    SBTexturePool* skyBoxTexturePoolObj;
                } resource;
            } m_sandBoxInfo;

            void configScene           (void);
            void configRendererCore    (void);
            void configRendererSPass   (void);      /* Shadow   (S) pass */
            void configRendererGPass   (void);      /* Geometry (G) pass */
            void configRendererFPass   (void);      /* Forward  (F) pass */
            void configRendererGuiPass (void);      /* Gui          pass */
            void configRendererOps     (void);
            void destroyRenderer       (void);

        public:
            SBImpl (void) {
                m_sandBoxInfo = {};

                m_sandBoxInfo.resource.sceneObj             = new Scene::SNImpl();
                m_sandBoxInfo.resource.sceneObj->initSceneInfo();
                m_sandBoxInfo.resource.collectionObj        = new Collection::CNImpl();
                m_sandBoxInfo.resource.collectionObj->initCollectionInfo();
                m_sandBoxInfo.resource.stdTexturePoolObj    = new SBTexturePool();
                m_sandBoxInfo.resource.stdTexturePoolObj->initTexturePoolInfo();
                m_sandBoxInfo.resource.skyBoxTexturePoolObj = new SBTexturePool();
                m_sandBoxInfo.resource.skyBoxTexturePoolObj->initTexturePoolInfo();

                /* Add default textures */
                m_sandBoxInfo.resource.stdTexturePoolObj->addTexture ("Asset/Texture/[D]_Simple_#FF00FF.png");  /* #0 */
                m_sandBoxInfo.resource.stdTexturePoolObj->addTexture ("Asset/Texture/[S]_Simple_#000000.png");  /* #1 */
                m_sandBoxInfo.resource.stdTexturePoolObj->addTexture ("Asset/Texture/[D]_Gui_No_Info.png");     /* #2 */
                /* Add sky box textures. The textures that make up the sky box are laid out in the below pattern. If you
                 * fold the 6 sides into a cube you'd get the completely textured cube that simulates a large landscape
                 *                              +-----------+
                 *                              |           |
                 *                              |     PY    |
                 *                              |           |
                 *                  +-----------+-----------+-----------+-----------+
                 *                  |           |           |           |           |
                 *                  |     NX    |     PZ    |     PX    |     NZ    |
                 *                  |           |           |           |           |
                 *                  +-----------+-----------+-----------+-----------+
                 *                              |           |
                 *                              |     NY    |
                 *                              |           |
                 *                              +-----------+
                */
                m_sandBoxInfo.resource.skyBoxTexturePoolObj->addTexture ("Asset/Texture/[D]_Sky_Box_PX.png");
                m_sandBoxInfo.resource.skyBoxTexturePoolObj->addTexture ("Asset/Texture/[D]_Sky_Box_NX.png");
                m_sandBoxInfo.resource.skyBoxTexturePoolObj->addTexture ("Asset/Texture/[D]_Sky_Box_PY.png");
                m_sandBoxInfo.resource.skyBoxTexturePoolObj->addTexture ("Asset/Texture/[D]_Sky_Box_NY.png");
                m_sandBoxInfo.resource.skyBoxTexturePoolObj->addTexture ("Asset/Texture/[D]_Sky_Box_PZ.png");
                m_sandBoxInfo.resource.skyBoxTexturePoolObj->addTexture ("Asset/Texture/[D]_Sky_Box_NZ.png");
            }

            void initSandBoxInfo (void) {
                auto& meta             = m_sandBoxInfo.meta;
                auto& resource         = m_sandBoxInfo.resource;
                /* Note that, larger the resolution of the shadow image, the higher the quality of shadows. But of course
                 * this comes with an additional computing cost, so you need to balance quality and performance for your
                 * particular target
                */
                meta.shadowImageWidth  = 1024;
                meta.shadowImageHeight = 1024;
                /* Configs */
                configScene();
                configRendererCore();
                configRendererSPass();
                configRendererGPass();
                configRendererFPass();
                configRendererGuiPass();
                configRendererOps();
                /* Reports */
                resource.sceneObj->generateReport();
                resource.collectionObj->generateReport();
                resource.stdTexturePoolObj->generateReport();
                resource.skyBoxTexturePoolObj->generateReport();
            }

            void runSandBox (void) {
                auto& meta                       = m_sandBoxInfo.meta;
                auto& resource                   = m_sandBoxInfo.resource;
                auto& shadow                     = g_systemConfig.shadow;

                auto windowObj                   = resource.collectionObj->getCollectionTypeInstance
                                                   <Renderer::VKWindow>    ("CORE");
                auto logDeviceObj                = resource.collectionObj->getCollectionTypeInstance
                                                   <Renderer::VKLogDevice> ("CORE");
                auto rendererObj                 = resource.collectionObj->getCollectionTypeInstance
                                                   <Renderer::VKRenderer>  ("DRAW_OPS");
                auto guiObj                      = resource.collectionObj->getCollectionTypeInstance
                                                   <Renderer::VKGui>       ("DRAW_OPS");

                auto skyBoxTransformComponent    = resource.sceneObj->getComponent <TransformComponent> (meta.skyBoxEntity);
                auto stdMeshInstanceBatchingObj  = resource.sceneObj->getSystem    <SYStdMeshInstanceBatching>();
                auto wireMeshInstanceBatchingObj = resource.sceneObj->getSystem    <SYWireMeshInstanceBatching>();
                auto lightInstanceBatchingObj    = resource.sceneObj->getSystem    <SYLightInstanceBatching>();
                auto cameraControllerObj         = resource.sceneObj->getSystem    <SYCameraController>();
                auto sceneViewObj                = resource.sceneObj->getSystem    <SYSceneView>();
                auto entityCollectionViewObj     = resource.sceneObj->getSystem    <SYEntityCollectionView>();
                auto componentEditorViewObj      = resource.sceneObj->getSystem    <SYComponentEditorView>();
                auto configViewObj               = resource.sceneObj->getSystem    <SYConfigView>();
                auto shadowRenderingObj          = resource.sceneObj->getSystem    <SYShadowRendering>();
                auto shadowCubeRenderingObj      = resource.sceneObj->getSystem    <SYShadowCubeRendering>();
                auto gDefaultRenderingObj        = resource.sceneObj->getSystem    <SYGDefaultRendering>();
                auto lightRenderingObj           = resource.sceneObj->getSystem    <SYLightRendering>();
                auto wireRenderingObj            = resource.sceneObj->getSystem    <SYWireRendering>();
                auto skyBoxRenderingObj          = resource.sceneObj->getSystem    <SYSkyBoxRendering>();
                auto fDefaultRenderingObj        = resource.sceneObj->getSystem    <SYFDefaultRendering>();
                auto debugRenderingObj           = resource.sceneObj->getSystem    <SYDebugRendering>();
                auto guiRenderingObj             = resource.sceneObj->getSystem    <SYGuiRendering>();

                auto activeLights                = lightInstanceBatchingObj->getBatchedActiveLights();
                auto lightTypeOffsets            = lightInstanceBatchingObj->getLightTypeOffsets();
                uint32_t otherLightsCount        = lightTypeOffsets->pointLightsOffset;
                uint32_t pointLightsCount        = lightTypeOffsets->lightsCount - lightTypeOffsets->pointLightsOffset;
                float frameDelta                 = 0.0f;

                /* Un-batched buffer data */
                MeshInstanceUBO skyBoxMeshInstance;
                ShadowConfigUBO shadowConfig;

                /* Some systems need to be initialized post-renderer-config */
                cameraControllerObj->initCameraControllerInfo         (
                    resource.sceneObj,
                    resource.collectionObj
                );
                sceneViewObj->initSceneViewInfo                       (
                    resource.collectionObj
                );
                entityCollectionViewObj->initEntityCollectionViewInfo (
                    meta.activeCameraEntity,                                /* Default selected entity */
                    meta.parentEntityToChildrenMap,
                    resource.sceneObj
                );
                componentEditorViewObj->initComponentEditorViewInfo   (
                    resource.sceneObj,
                    resource.collectionObj
                );
                configViewObj->initConfigViewInfo                     (
                    resource.collectionObj,
                    resource.stdTexturePoolObj
                );
                shadowRenderingObj->initShadowRenderingInfo           (
                    static_cast <uint32_t> (activeLights.size()),           /* Custom frame buffers count */
                    resource.sceneObj,
                    resource.collectionObj
                );
                shadowCubeRenderingObj->initShadowCubeRenderingInfo   (
                    static_cast <uint32_t> (activeLights.size()),           /* Custom frame buffers count */
                    resource.sceneObj,
                    resource.collectionObj
                );
                gDefaultRenderingObj->initGDefaultRenderingInfo       (
                    resource.sceneObj,
                    resource.collectionObj
                );
                lightRenderingObj->initLightRenderingInfo             (
                    resource.collectionObj
                );
                wireRenderingObj->initWireRenderingInfo               (
                    resource.sceneObj,
                    resource.collectionObj
                );
                skyBoxRenderingObj->initSkyBoxRenderingInfo           (
                    resource.sceneObj,
                    resource.collectionObj
                );
                fDefaultRenderingObj->initFDefaultRenderingInfo       (
                    resource.sceneObj,
                    resource.collectionObj
                );
                debugRenderingObj->initDebugRenderingInfo             (
                    resource.collectionObj
                );
                guiRenderingObj->initGuiRenderingInfo                 (
                    resource.collectionObj
                );

                while (!windowObj->isWindowClosed()) {
                    glfwPollEvents();
                    auto startOfFrameTime = PROFILE_CAPTURE;

                    {   /* Data update */
                        /* Handle window events */
                        resource.collectionObj->updateCollectionType <Renderer::VKWindow>();

                        /* Controller updates */
                        cameraControllerObj->update (frameDelta, meta.activeCameraEntity);

                        /* Batched updates */
                        stdMeshInstanceBatchingObj->update();
                        wireMeshInstanceBatchingObj->update();
                        lightInstanceBatchingObj->update (meta.shadowImageWidth /
                                                          static_cast <float> (meta.shadowImageHeight));

                        /* Un-batched updates */
                        skyBoxMeshInstance.modelMatrix = skyBoxTransformComponent->createModelMatrix();
                        shadowConfig.minShadowFactor   = shadow.minShadowFactor;
                        shadowConfig.minDepthBias      = shadow.minDepthBias;
                        shadowConfig.maxDepthBias      = shadow.maxDepthBias;
                    }
                    /* Frame update */
                    if (rendererObj->beginFrame()) {
                        {   /* S pass */
                            size_t activeLightIdx = 0;

                            for (uint32_t i = 0; i < otherLightsCount; i++) {
                                shadowRenderingObj->update (
                                    static_cast <uint32_t> (activeLightIdx),
                                    stdMeshInstanceBatchingObj->getBatchedMeshInstancesLite (TAG_TYPE_STD_NO_ALPHA).data(),
                                    &lightInstanceBatchingObj->getBatchedActiveLights()[activeLightIdx]
                                );
                                ++activeLightIdx;
                            }

                            for (uint32_t i = 0; i < pointLightsCount; i++) {
                            for (uint32_t cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx++) {
                                shadowCubeRenderingObj->update (
                                    static_cast <uint32_t> (activeLightIdx),
                                    stdMeshInstanceBatchingObj->getBatchedMeshInstancesLite (TAG_TYPE_STD_NO_ALPHA).data(),
                                    &lightInstanceBatchingObj->getBatchedActiveLights()[activeLightIdx]
                                );
                                ++activeLightIdx;
                            }
                            }
                        }
                        {   /* G pass */
                            gDefaultRenderingObj->update (
                                stdMeshInstanceBatchingObj->getBatchedMeshInstances (TAG_TYPE_STD_NO_ALPHA).data(),
                                cameraControllerObj->getActiveCamera()
                            );
                        }
                        {   /* F pass */
                            lightRenderingObj->update    (
                                lightInstanceBatchingObj->getBatchedLightInstances().data(),
                                &shadowConfig,
                                lightTypeOffsets,
                                cameraControllerObj->getActiveCamera()
                            );
                            wireRenderingObj->update     (
                                wireMeshInstanceBatchingObj->getBatchedMeshInstances().data(),
                                cameraControllerObj->getActiveCamera()
                            );
                            skyBoxRenderingObj->update   (
                                &skyBoxMeshInstance,
                                cameraControllerObj->getActiveCamera()
                            );
                            fDefaultRenderingObj->update (
                                stdMeshInstanceBatchingObj->getBatchedMeshInstances (TAG_TYPE_STD_ALPHA).data(),
                                cameraControllerObj->getActiveCamera()
                            );
                            debugRenderingObj->update();
                        }
                        {   /* Gui pass */
                            guiObj->beginFrame();
                            {
                                sceneViewObj->update (
                                    frameDelta
                                );
                                entityCollectionViewObj->update();
                                componentEditorViewObj->update (
                                    entityCollectionViewObj->getSelectedEntity(),
                                    meta.activeCameraEntity,
                                    entityCollectionViewObj->getSelectedComponentType()
                                );
                                configViewObj->update();
                            }
                            guiObj->endFrame();
                            guiRenderingObj->update();
                        }
                        rendererObj->endFrame();
                    }

                    auto endOfFrameTime = PROFILE_CAPTURE;
                    frameDelta          = PROFILE_COMPUTE (startOfFrameTime, endOfFrameTime);
                }
                /* Remember that all of the operations in the above render method are asynchronous. That means that when
                 * we exit the render loop, drawing and presentation operations may still be going on. Cleaning up the
                 * resources while that is happening is a bad idea. To fix that problem, we should wait for the logical
                 * device to finish operations before exiting the loop and destroying the window
                */
                vkDeviceWaitIdle (*logDeviceObj->getLogDevice());
            }

            void destroySandBox (void) {
                auto& resource = m_sandBoxInfo.resource;
                /* Configs */
                destroyRenderer();
                /* Reports */
                resource.sceneObj->generateReport();
                resource.collectionObj->generateReport();
                resource.stdTexturePoolObj->generateReport();
                resource.skyBoxTexturePoolObj->generateReport();
            }

            ~SBImpl (void) {
                auto& resource = m_sandBoxInfo.resource;
                delete resource.skyBoxTexturePoolObj;
                delete resource.stdTexturePoolObj;
                delete resource.collectionObj;
                delete resource.sceneObj;
            }
    };
}   // namespace SandBox