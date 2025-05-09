#pragma once
#include "../Backend/Common.h"
#include "../Backend/Scene/SNImpl.h"
#include "../Backend/Collection/CNImpl.h"
#include "../Backend/Renderer/VKWindow.h"
#include "../Backend/Renderer/VKLogDevice.h"
#include "../Backend/Renderer/VKRenderer.h"
#include "SBTexturePool.h"
#include "System/SYDefaultMeshInstanceBatching.h"
#include "System/SYWireMeshInstanceBatching.h"
#include "System/SYDefaultLightInstanceBatching.h"
#include "System/SYCameraController.h"
#include "System/SYDefaultRendering.h"
#include "System/SYWireRendering.h"
#include "System/SYSkyBoxRendering.h"
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
                    Scene::Entity skyBoxEntity;
                } meta;

                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Collection::CNImpl* collectionObj;
                    SBTexturePool* defaultTexturePoolObj;
                    SBTexturePool* skyBoxTexturePoolObj;
                } resource;
            } m_sandBoxInfo;

            void configScene     (void);
            void configRenderer  (void);
            void destroyRenderer (void);

        public:
            SBImpl (void) {
                m_sandBoxInfo = {};

                m_sandBoxInfo.resource.sceneObj              = new Scene::SNImpl();
                m_sandBoxInfo.resource.sceneObj->initSceneInfo();
                m_sandBoxInfo.resource.collectionObj         = new Collection::CNImpl();
                m_sandBoxInfo.resource.collectionObj->initCollectionInfo();
                m_sandBoxInfo.resource.defaultTexturePoolObj = new SBTexturePool();
                m_sandBoxInfo.resource.defaultTexturePoolObj->initTexturePoolInfo();
                m_sandBoxInfo.resource.skyBoxTexturePoolObj  = new SBTexturePool();
                m_sandBoxInfo.resource.skyBoxTexturePoolObj->initTexturePoolInfo();

                auto& defaultTexturePoolObj = m_sandBoxInfo.resource.defaultTexturePoolObj;
                auto& skyBoxTexturePoolObj  = m_sandBoxInfo.resource.skyBoxTexturePoolObj;
                /* Add default textures */
                defaultTexturePoolObj->addTextureToPool ("Asset/Texture/[D]_Empty.png");    /* Texture idx 0 */
                defaultTexturePoolObj->addTextureToPool ("Asset/Texture/[S]_Empty.png");    /* Texture idx 1 */
                defaultTexturePoolObj->addTextureToPool ("Asset/Texture/[E]_Empty.png");    /* Texture idx 2 */
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
                /* Configs */
                configScene();
                configRenderer();
                /* Reports */
                m_sandBoxInfo.resource.sceneObj->generateReport();
                m_sandBoxInfo.resource.collectionObj->generateReport();
                m_sandBoxInfo.resource.defaultTexturePoolObj->generateReport();
                m_sandBoxInfo.resource.skyBoxTexturePoolObj->generateReport();
            }

            void runSandBox (void) {
                auto& skyBoxEntity                   = m_sandBoxInfo.meta.skyBoxEntity;
                auto& sceneObj                       = m_sandBoxInfo.resource.sceneObj;
                auto& collectionObj                  = m_sandBoxInfo.resource.collectionObj;

                auto skyBoxTransformComponent        = sceneObj->getComponent <TransformComponent> (skyBoxEntity);
                auto defaultMeshInstanceBatchingObj  = sceneObj->getSystem    <SYDefaultMeshInstanceBatching>();
                auto wireMeshInstanceBatchingObj     = sceneObj->getSystem    <SYWireMeshInstanceBatching>();
                auto defaultLightInstanceBatchingObj = sceneObj->getSystem    <SYDefaultLightInstanceBatching>();
                auto cameraControllerObj             = sceneObj->getSystem    <SYCameraController>();
                auto defaultRenderingObj             = sceneObj->getSystem    <SYDefaultRendering>();
                auto wireRenderingObj                = sceneObj->getSystem    <SYWireRendering>();
                auto skyBoxRenderingObj              = sceneObj->getSystem    <SYSkyBoxRendering>();
                float frameDelta                     = 0.0f;
                /* Un-batched mesh instance */
                MeshInstanceUBO skyBoxMeshInstance;
                /* Note that, certain systems may need some collection type instances to be configured before it is
                 * initialised. Hence why their init methods are called here instead of in the scene config
                */
                cameraControllerObj->initCameraControllerInfo (sceneObj, collectionObj);
                defaultRenderingObj->initDefaultRenderingInfo (sceneObj, collectionObj);
                wireRenderingObj->initWireRenderingInfo       (sceneObj, collectionObj);
                skyBoxRenderingObj->initSkyBoxRenderingInfo   (sceneObj, collectionObj);

                auto windowObj    = collectionObj->getCollectionTypeInstance <Renderer::VKWindow>    ("DEFAULT");
                auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
                auto rendererObj  = collectionObj->getCollectionTypeInstance <Renderer::VKRenderer>  ("DEFAULT");

                while (!windowObj->isWindowClosed()) {
                    glfwPollEvents();
                    auto startOfFrameTime = PROFILE_CAPTURE;
                    /* Handle window events */
                    collectionObj->updateCollectionType <Renderer::VKWindow>();
                    /* Controller updates */
                    cameraControllerObj->update (frameDelta);
                    /* Batching updates */
                    defaultMeshInstanceBatchingObj->update();
                    wireMeshInstanceBatchingObj->update();
                    defaultLightInstanceBatchingObj->update();
                    /* Un-batched updates */
                    skyBoxMeshInstance.modelMatrix = skyBoxTransformComponent->createModelMatrix();

                    if (rendererObj->beginFrame()) {
                        skyBoxRenderingObj->update  (&skyBoxMeshInstance,
                                                     cameraControllerObj->getActiveCamera());
                        wireRenderingObj->update    (wireMeshInstanceBatchingObj->getBatchedMeshInstances().data(),
                                                     cameraControllerObj->getActiveCamera());
                        defaultRenderingObj->update (defaultMeshInstanceBatchingObj->getBatchedMeshInstances().data(),
                                                     defaultLightInstanceBatchingObj->getBatchedLightInstances().data(),
                                                     defaultLightInstanceBatchingObj->getLightTypeOffsets(),
                                                     cameraControllerObj->getActiveCamera());
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
                m_sandBoxInfo.resource.defaultTexturePoolObj->generateReport();
                m_sandBoxInfo.resource.skyBoxTexturePoolObj->generateReport();
            }

            ~SBImpl (void) {
                delete m_sandBoxInfo.resource.sceneObj;
                delete m_sandBoxInfo.resource.collectionObj;
                delete m_sandBoxInfo.resource.defaultTexturePoolObj;
                delete m_sandBoxInfo.resource.skyBoxTexturePoolObj;
            }
    };
}   // namespace SandBox