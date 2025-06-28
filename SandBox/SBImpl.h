#pragma once
#include "../Backend/Common.h"
#include "../Backend/Scene/SNImpl.h"
#include "../Backend/Collection/CNImpl.h"
#include "../Backend/Renderer/VKWindow.h"
#include "../Backend/Renderer/VKLogDevice.h"
#include "../Backend/Renderer/VKRenderer.h"
#include "SBTexturePool.h"
#include "System/Batching/SYStdMeshInstanceBatching.h"
#include "System/Batching/SYWireMeshInstanceBatching.h"
#include "System/Batching/SYLightInstanceBatching.h"
#include "System/Controller/SYCameraController.h"
#include "System/Rendering/SYShadowRendering.h"
#include "System/Rendering/SYShadowCubeRendering.h"
#include "System/Rendering/SYGDefaultRendering.h"
#include "System/Rendering/SYLightRendering.h"
#include "System/Rendering/SYWireRendering.h"
#include "System/Rendering/SYSkyBoxRendering.h"
#include "System/Rendering/SYFDefaultRendering.h"
#include "System/Rendering/SYDebugRendering.h"
#include "../Backend/Scene/SNType.h"
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
                } meta;

                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Collection::CNImpl* collectionObj;
                    SBTexturePool* stdTexturePoolObj;
                    SBTexturePool* skyBoxTexturePoolObj;
                } resource;
            } m_sandBoxInfo;

            void configScene          (void);
            void configRendererCore   (void);
            void configRendererCommon (void);
            void configRendererSPass  (void);   /* Shadow   (S) pass */
            void configRendererGPass  (void);   /* Geometry (G) pass */
            void configRendererFPass  (void);   /* Forward  (F) pass */
            void configRendererOps    (void);
            void destroyRenderer      (void);

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

                auto& stdTexturePoolObj    = m_sandBoxInfo.resource.stdTexturePoolObj;
                auto& skyBoxTexturePoolObj = m_sandBoxInfo.resource.skyBoxTexturePoolObj;
                /* Add default textures */
                stdTexturePoolObj->addTextureToPool ("Asset/Texture/[D]_Empty.png");    /* Texture idx 0 */
                stdTexturePoolObj->addTextureToPool ("Asset/Texture/[S]_Empty.png");    /* Texture idx 1 */
                stdTexturePoolObj->addTextureToPool ("Asset/Texture/[E]_Empty.png");    /* Texture idx 2 */
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
                skyBoxTexturePoolObj->addTextureToPool ("Asset/Texture/[D]_Sky_Box_PX.png");
                skyBoxTexturePoolObj->addTextureToPool ("Asset/Texture/[D]_Sky_Box_NX.png");
                skyBoxTexturePoolObj->addTextureToPool ("Asset/Texture/[D]_Sky_Box_PY.png");
                skyBoxTexturePoolObj->addTextureToPool ("Asset/Texture/[D]_Sky_Box_NY.png");
                skyBoxTexturePoolObj->addTextureToPool ("Asset/Texture/[D]_Sky_Box_PZ.png");
                skyBoxTexturePoolObj->addTextureToPool ("Asset/Texture/[D]_Sky_Box_NZ.png");
            }

            void initSandBoxInfo (void) {
                /* Note that, larger the resolution of the shadow/depth image, the higher the quality of shadows. But of
                 * course this comes with an additional computing cost, so you need to balance quality and performance for
                 * your particular target
                */
                m_sandBoxInfo.meta.shadowImageWidth  = 512;
                m_sandBoxInfo.meta.shadowImageHeight = 512;
                /* Configs */
                configScene();
                configRendererCore();
                configRendererCommon();
                configRendererSPass();
                configRendererGPass();
                configRendererFPass();
                configRendererOps();
                /* Reports */
                m_sandBoxInfo.resource.sceneObj->generateReport();
                m_sandBoxInfo.resource.collectionObj->generateReport();
                m_sandBoxInfo.resource.stdTexturePoolObj->generateReport();
                m_sandBoxInfo.resource.skyBoxTexturePoolObj->generateReport();
            }

            void runSandBox (void) {
                auto& shadowImageWidth           = m_sandBoxInfo.meta.shadowImageWidth;
                auto& shadowImageHeight          = m_sandBoxInfo.meta.shadowImageHeight;
                auto& skyBoxEntity               = m_sandBoxInfo.meta.skyBoxEntity;
                auto& sceneObj                   = m_sandBoxInfo.resource.sceneObj;
                auto& collectionObj              = m_sandBoxInfo.resource.collectionObj;

                auto skyBoxTransformComponent    = sceneObj->getComponent <TransformComponent> (skyBoxEntity);
                auto stdMeshInstanceBatchingObj  = sceneObj->getSystem    <SYStdMeshInstanceBatching>();
                auto wireMeshInstanceBatchingObj = sceneObj->getSystem    <SYWireMeshInstanceBatching>();
                auto lightInstanceBatchingObj    = sceneObj->getSystem    <SYLightInstanceBatching>();
                auto cameraControllerObj         = sceneObj->getSystem    <SYCameraController>();
                auto shadowRenderingObj          = sceneObj->getSystem    <SYShadowRendering>();
                auto shadowCubeRenderingObj      = sceneObj->getSystem    <SYShadowCubeRendering>();
                auto gDefaultRenderingObj        = sceneObj->getSystem    <SYGDefaultRendering>();
                auto lightRenderingObj           = sceneObj->getSystem    <SYLightRendering>();
                auto wireRenderingObj            = sceneObj->getSystem    <SYWireRendering>();
                auto skyBoxRenderingObj          = sceneObj->getSystem    <SYSkyBoxRendering>();
                auto fDefaultRenderingObj        = sceneObj->getSystem    <SYFDefaultRendering>();
                auto debugRenderingObj           = sceneObj->getSystem    <SYDebugRendering>();

                auto activeLights                = lightInstanceBatchingObj->getBatchedActiveLights();
                auto lightTypeOffsets            = lightInstanceBatchingObj->getLightTypeOffsets();
                uint32_t otherLightsCount        = lightTypeOffsets->pointLightsOffset;
                uint32_t pointLightsCount        = lightTypeOffsets->lightsCount - lightTypeOffsets->pointLightsOffset;
                float frameDelta                 = 0.0f;
                /* Un-batched mesh instance */
                MeshInstanceUBO skyBoxMeshInstance;
                /* Note that, certain systems may need some collection type instances to be configured before it is
                 * initialised. Hence why their init methods are called post-renderer-config
                */
                cameraControllerObj->initCameraControllerInfo       (sceneObj, collectionObj);
                shadowRenderingObj->initShadowRenderingInfo         (
                    static_cast <uint32_t> (activeLights.size()),   /* Custom frame buffers count */
                    sceneObj,
                    collectionObj
                );
                shadowCubeRenderingObj->initShadowCubeRenderingInfo (
                    static_cast <uint32_t> (activeLights.size()),   /* Custom frame buffers count */
                    sceneObj,
                    collectionObj
                );
                gDefaultRenderingObj->initGDefaultRenderingInfo     (sceneObj, collectionObj);
                lightRenderingObj->initLightRenderingInfo           (          collectionObj);
                wireRenderingObj->initWireRenderingInfo             (sceneObj, collectionObj);
                skyBoxRenderingObj->initSkyBoxRenderingInfo         (sceneObj, collectionObj);
                fDefaultRenderingObj->initFDefaultRenderingInfo     (sceneObj, collectionObj);
                debugRenderingObj->initDebugRenderingInfo           (          collectionObj);

                auto windowObj    = collectionObj->getCollectionTypeInstance <Renderer::VKWindow>    ("CORE");
                auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("CORE");
                auto rendererObj  = collectionObj->getCollectionTypeInstance <Renderer::VKRenderer>  ("DRAW_OPS");

                while (!windowObj->isWindowClosed()) {
                    glfwPollEvents();
                    auto startOfFrameTime = PROFILE_CAPTURE;

                    /* Handle window events */
                    collectionObj->updateCollectionType <Renderer::VKWindow>();
                    /* Controller updates */
                    cameraControllerObj->update (frameDelta);
                    /* Batching updates */
                    stdMeshInstanceBatchingObj->update();
                    wireMeshInstanceBatchingObj->update();
                    lightInstanceBatchingObj->update (shadowImageWidth / static_cast <float> (shadowImageHeight));
                    /* Un-batched updates */
                    skyBoxMeshInstance.modelMatrix = skyBoxTransformComponent->createModelMatrix();

                    if (rendererObj->beginFrame()) {
                        {   /* S pass */
                            size_t activeLightIdx = 0;

                            for (uint32_t i = 0; i < otherLightsCount; i++) {
                                shadowRenderingObj->update (
                                    static_cast <uint32_t> (activeLightIdx),
                                    stdMeshInstanceBatchingObj->getBatchedMeshInstancesLite (TAG_TYPE_STD).data(),
                                    &lightInstanceBatchingObj->getBatchedActiveLights()[activeLightIdx]
                                );
                                ++activeLightIdx;
                            }
                            for (uint32_t i = 0; i < pointLightsCount; i++) {
                            for (uint32_t cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx++) {
                                shadowCubeRenderingObj->update (
                                    static_cast <uint32_t> (activeLightIdx),
                                    stdMeshInstanceBatchingObj->getBatchedMeshInstancesLite (TAG_TYPE_STD).data(),
                                    &lightInstanceBatchingObj->getBatchedActiveLights()[activeLightIdx]
                                );
                                ++activeLightIdx;
                            }
                            }
                        }
                        {   /* G pass */
                            gDefaultRenderingObj->update (
                                stdMeshInstanceBatchingObj->getBatchedMeshInstances (TAG_TYPE_STD).data(),
                                cameraControllerObj->getActiveCamera()
                            );
                        }
                        {   /* F pass */
                            lightRenderingObj->update    (
                                lightInstanceBatchingObj->getBatchedLightInstances().data(),
                                lightTypeOffsets,        /* Stale data */
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
                /* Configs */
                destroyRenderer();
                /* Reports */
                m_sandBoxInfo.resource.sceneObj->generateReport();
                m_sandBoxInfo.resource.collectionObj->generateReport();
                m_sandBoxInfo.resource.stdTexturePoolObj->generateReport();
                m_sandBoxInfo.resource.skyBoxTexturePoolObj->generateReport();
            }

            ~SBImpl (void) {
                delete m_sandBoxInfo.resource.sceneObj;
                delete m_sandBoxInfo.resource.collectionObj;
                delete m_sandBoxInfo.resource.stdTexturePoolObj;
                delete m_sandBoxInfo.resource.skyBoxTexturePoolObj;
            }
    };
}   // namespace SandBox