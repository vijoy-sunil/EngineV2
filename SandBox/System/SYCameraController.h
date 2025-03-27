#pragma once
#include <glm/glm.hpp>
#include <stdexcept>
#include "../../Backend/Scene/SNSystemBase.h"
#include "../../Backend/Scene/SNImpl.h"
#include "../../Backend/Collection/CNImpl.h"
#include "../../Backend/Log/LGImpl.h"
#include "../../Backend/Renderer/VKSwapChain.h"
#include "../SBComponentType.h"
#include "../SBRendererType.h"

namespace SandBox {
    class SYCameraController: public Scene::SNSystemBase {
        private:
            struct CameraControllerInfo {
                struct Meta {
                    Renderer::VKSwapChain* swapChainObj;
                    ActiveCameraPC activeCamera;
                } meta;

                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Log::LGImpl* logObj;
                } resource;
            } m_cameraControlerInfo;

        public:
            SYCameraController (void) {
                m_cameraControlerInfo = {};

                auto& logObj = m_cameraControlerInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",    __FILE__);
                logObj->updateLogConfig (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
            }

            void initCameraControllerInfo (Scene::SNImpl* sceneObj,
                                           Collection::CNImpl* collectionObj) {

                if (sceneObj == nullptr || collectionObj == nullptr) {
                    LOG_ERROR (m_cameraControlerInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                      << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }

                m_cameraControlerInfo.meta.swapChainObj =
                collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain> (
                    "DEFAULT"
                );
                m_cameraControlerInfo.meta.activeCamera = {};
                m_cameraControlerInfo.resource.sceneObj = sceneObj;
            }

            ActiveCameraPC* getActiveCamera (void) {
                return &m_cameraControlerInfo.meta.activeCamera;
            }

            void update (void) {
                auto& swapChainObj = m_cameraControlerInfo.meta.swapChainObj;
                auto& activeCamera = m_cameraControlerInfo.meta.activeCamera;
                auto& sceneObj     = m_cameraControlerInfo.resource.sceneObj;

                for (auto const& entity: m_entities) {
                    auto cameraComponent    = sceneObj->getComponent <CameraComponent>    (entity);
                    auto transformComponent = sceneObj->getComponent <TransformComponent> (entity);

                    if (!cameraComponent->m_active)
                        continue;
                    /* Compute aspect ratio */
                    float aspectRatio             = swapChainObj->getSwapChainExtent()->width/
                                                    static_cast <float> (swapChainObj->getSwapChainExtent()->height);
                    activeCamera.viewMatrix       = glm::inverse (transformComponent->getModelMatrix());
                    activeCamera.projectionMatrix = cameraComponent->getProjectionMatrix (aspectRatio);
                }
            }

            ~SYCameraController (void) {
                delete m_cameraControlerInfo.resource.logObj;
            }
    };
}   // namespace SandBox