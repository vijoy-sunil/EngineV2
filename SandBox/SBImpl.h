#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <chrono>
#include "../Backend/Scene/SNImpl.h"
#include "../Backend/Collection/CNImpl.h"
#include "../Backend/Renderer/VKWindow.h"
#include "../Backend/Renderer/VKLogDevice.h"
#include "../Backend/Renderer/VKRenderer.h"
#include "SBTexturePool.h"
#include "System/SYMeshInstanceBatching.h"
#include "System/SYLightInstanceBatching.h"
#include "System/SYCameraController.h"
#include "System/SYDefaultRendering.h"

#define PROFILE_CAPTURE                 std::chrono::high_resolution_clock::now()
#define PROFILE_COMPUTE(begin, end)     std::chrono::duration <float, std::chrono::seconds::period> \
                                        (end - begin).count()

namespace SandBox {
    class SBImpl {
        private:
            struct SandBoxInfo {
                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Collection::CNImpl* collectionObj;
                    SBTexturePool* defaultTexturePoolObj;
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

                /* Add default textures */
                m_sandBoxInfo.resource.defaultTexturePoolObj->addTextureToPool (
                    "Asset/Texture/[D]_Empty.png"   /* Texture idx 0 */
                );
                m_sandBoxInfo.resource.defaultTexturePoolObj->addTextureToPool (
                    "Asset/Texture/[S]_Empty.png"   /* Texture idx 1 */
                );
                m_sandBoxInfo.resource.defaultTexturePoolObj->addTextureToPool (
                    "Asset/Texture/[E]_Empty.png"   /* Texture idx 2 */
                );
            }

            void initSandBoxInfo (void) {
                /* Configs */
                configScene();
                configRenderer();
                /* Reports */
                m_sandBoxInfo.resource.sceneObj->generateReport();
                m_sandBoxInfo.resource.collectionObj->generateReport();
                m_sandBoxInfo.resource.defaultTexturePoolObj->generateReport();
            }

            void runSandBox (void) {
                auto& sceneObj                = m_sandBoxInfo.resource.sceneObj;
                auto& collectionObj           = m_sandBoxInfo.resource.collectionObj;
                auto meshInstanceBatchingObj  = sceneObj->getSystem <SYMeshInstanceBatching>();
                auto lightInstanceBatchingObj = sceneObj->getSystem <SYLightInstanceBatching>();
                auto cameraControllerObj      = sceneObj->getSystem <SYCameraController>();
                auto defaultRenderingObj      = sceneObj->getSystem <SYDefaultRendering>();
                float frameDelta              = 0.0f;
                /* Note that, certain systems may need some collection type instances to be configured before it is
                 * initialised. Hence why their init methods are called here instead of in the scene config
                */
                cameraControllerObj->initCameraControllerInfo (sceneObj, collectionObj);
                defaultRenderingObj->initDefaultRenderingInfo (sceneObj, collectionObj);

                auto windowObj                = collectionObj->getCollectionTypeInstance <Renderer::VKWindow>    (
                    "DEFAULT"
                );
                auto logDeviceObj             = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> (
                    "DEFAULT"
                );
                auto rendererObj              = collectionObj->getCollectionTypeInstance <Renderer::VKRenderer>  (
                    "DEFAULT"
                );

                while (!windowObj->isWindowClosed()) {
                    glfwPollEvents();
                    auto startOfFrameTime = PROFILE_CAPTURE;
                    /* Handle window events */
                    collectionObj->updateCollectionType <Renderer::VKWindow>();
                    /* Controller updates */
                    cameraControllerObj->update (frameDelta);
                    /* Batching updates */
                    meshInstanceBatchingObj->update();
                    lightInstanceBatchingObj->update();

                    if (rendererObj->beginFrame()) {
                        defaultRenderingObj->update (
                            meshInstanceBatchingObj->getBatchedMeshInstances().data(),
                            lightInstanceBatchingObj->getBatchedLightInstances().data(),
                            lightInstanceBatchingObj->getLightTypeOffsets(),
                            cameraControllerObj->getActiveCamera()
                        );
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
            }

            ~SBImpl (void) {
                delete m_sandBoxInfo.resource.sceneObj;
                delete m_sandBoxInfo.resource.collectionObj;
                delete m_sandBoxInfo.resource.defaultTexturePoolObj;
            }
    };
}   // namespace SandBox