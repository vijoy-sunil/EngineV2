#pragma once
#include "../../Backend/Common.h"
#include "../../Backend/Scene/SNSystemBase.h"
#include "../../Backend/Scene/SNImpl.h"
#include "../../Backend/Collection/CNImpl.h"
#include "../../Backend/Log/LGImpl.h"
#include "../../Backend/Renderer/VKWindow.h"
#include "../../Backend/Renderer/VKSwapChain.h"
#include "SYEnum.h"
#include "../SBComponentType.h"
#include "../SBRendererType.h"

namespace SandBox {
    class SYCameraController: public Scene::SNSystemBase {
        private:
            struct CameraControllerInfo {
                struct Meta {
                    float lastCursorXPos;
                    float lastCursorYPos;
                    /* Delta values are computed in their respective bindings, and then used in the update function */
                    float pitchDelta;
                    float yawDelta;
                    float rollDelta;
                    float lateralDelta;
                    float axialDelta;
                    float fovDelta;

                    float minFov;
                    float maxFov;

                    float cursorSensitivity;
                    float scrollSensitivity;
                    float movementSensitivity;
                    float fovSensitivity;
                    float deltaDamp;
                    float levelDamp;

                    ActiveCameraPC activeCamera;
                } meta;

                struct State {
                    e_keyState droneToggle;
                    e_keyState fineToggle;
                    e_keyState levelToggle;
                    /* When the cursor position binding is called for the first time, the cursor X and Y position is
                     * equal to the location your cursor entered the screen from. This is often a position that is
                     * significantly far away from the center of the screen, resulting in large offsets and thus a large
                     * movement jump. We can circumvent this issue by defining a boolean variable to check if this is
                     * the first time we receive cursor input. If it is, we update the initial cursor position to the
                     * new X and Y values. The resulting cursor movements will then use the newly entered position
                     * coordinates to calculate the offsets
                    */
                    bool firstCursorPositionEvent;
                    bool levelingDisabled;
                } state;

                struct Resource {
                    Scene::SNImpl* sceneObj;
                    Log::LGImpl* logObj;
                    Renderer::VKWindow* windowObj;
                    Renderer::VKSwapChain* swapChainObj;
                } resource;
            } m_cameraControllerInfo;

            void setCameraControllerSensitivity (const e_sensitivityType sensitivityType) {
                auto& meta = m_cameraControllerInfo.meta;
                if (sensitivityType         == SENSITIVITY_TYPE_FINE) {
                    meta.cursorSensitivity   = 0.03f;
                    meta.scrollSensitivity   = 0.25f;
                    meta.movementSensitivity = 1.00f;
                    meta.fovSensitivity      = 0.10f;
                    meta.deltaDamp           = 0.00f;
                }
                else {
                    meta.cursorSensitivity   = 0.08f;
                    meta.scrollSensitivity   = 0.90f;
                    meta.movementSensitivity = 6.50f;
                    meta.fovSensitivity      = 0.40f;
                    meta.deltaDamp           = 0.85f;
                }
            }

            void setCameraControllerBindings (void) {
                auto& windowObj = m_cameraControllerInfo.resource.windowObj;

                windowObj->setKeyEventBinding (GLFW_KEY_PERIOD,
                    [this](void) {
                        auto& state            = m_cameraControllerInfo.state;
                        if (state.droneToggle == KEY_STATE_LOCKED)
                            state.droneToggle  = KEY_STATE_PENDING_UNLOCK;

                        if (state.droneToggle == KEY_STATE_UNLOCKED)
                            state.droneToggle  = KEY_STATE_PENDING_LOCK;
                    },
                    [this, windowObj](void) {
                        auto& state            = m_cameraControllerInfo.state;
                        if (state.droneToggle == KEY_STATE_PENDING_UNLOCK) {
                            state.droneToggle  = KEY_STATE_UNLOCKED;

                            state.firstCursorPositionEvent = true;
                            windowObj->toggleCursorPositionCallback (true);
                            windowObj->toggleScrollOffsetCallback   (true);
                        }

                        if (state.droneToggle == KEY_STATE_PENDING_LOCK) {
                            state.droneToggle  = KEY_STATE_LOCKED;

                            windowObj->toggleCursorPositionCallback (false);
                            windowObj->toggleScrollOffsetCallback   (false);
                        }
                    }
                );
                windowObj->setKeyEventBinding (GLFW_KEY_SLASH,
                    [this](void) {
                        auto& state           = m_cameraControllerInfo.state;
                        if (state.fineToggle == KEY_STATE_LOCKED) {
                            state.fineToggle  = KEY_STATE_UNLOCKED;

                            setCameraControllerSensitivity (SENSITIVITY_TYPE_FINE);
                        }
                    },
                    [this](void) {
                        auto& state           = m_cameraControllerInfo.state;
                        if (state.fineToggle == KEY_STATE_UNLOCKED) {
                            state.fineToggle  = KEY_STATE_LOCKED;

                            setCameraControllerSensitivity (SENSITIVITY_TYPE_COARSE);
                        }
                    }
                );
                windowObj->setKeyEventBinding (GLFW_KEY_COMMA,
                    [this](void) {
                        auto& state            = m_cameraControllerInfo.state;
                        if (state.levelToggle == KEY_STATE_LOCKED) {
                            state.levelToggle  = KEY_STATE_UNLOCKED;

                            state.levelingDisabled = false;
                        }
                    },
                    [this](void) {
                        auto& state            = m_cameraControllerInfo.state;
                        if (state.levelToggle == KEY_STATE_UNLOCKED) {
                            state.levelToggle  = KEY_STATE_LOCKED;

                            state.levelingDisabled = true;
                        }
                    }
                );
                windowObj->setCursorPositionBinding (
                    [this](const double xPos, const double yPos) {
                        auto& meta  = m_cameraControllerInfo.meta;
                        auto& state = m_cameraControllerInfo.state;
                        if (state.firstCursorPositionEvent) {
                            meta.lastCursorXPos            = static_cast <float> (xPos);
                            meta.lastCursorYPos            = static_cast <float> (yPos);
                            state.firstCursorPositionEvent = false;
                        }

                        meta.pitchDelta     = meta.cursorSensitivity *
                                              static_cast <float> (meta.lastCursorYPos - yPos);
                        meta.yawDelta       = meta.cursorSensitivity *
                                              static_cast <float> (meta.lastCursorXPos - xPos);
                        meta.lastCursorXPos = static_cast <float> (xPos);
                        meta.lastCursorYPos = static_cast <float> (yPos);
                    }
                );
                windowObj->setScrollOffsetBinding (
                    [this](const double xOffset, const double yOffset) {
                        static_cast <void> (yOffset);
                        auto& meta     = m_cameraControllerInfo.meta;
                        meta.rollDelta = meta.scrollSensitivity * static_cast <float> (xOffset);
                    }
                );
                windowObj->setKeyEventBinding (GLFW_KEY_D,
                    [this](void) {
                        auto& meta             = m_cameraControllerInfo.meta;
                        auto& state            = m_cameraControllerInfo.state;
                        if (state.droneToggle == KEY_STATE_UNLOCKED)
                            meta.lateralDelta  = meta.movementSensitivity;
                    }
                );
                windowObj->setKeyEventBinding (GLFW_KEY_A,
                    [this](void) {
                        auto& meta             = m_cameraControllerInfo.meta;
                        auto& state            = m_cameraControllerInfo.state;
                        if (state.droneToggle == KEY_STATE_UNLOCKED)
                            meta.lateralDelta  = -meta.movementSensitivity;
                    }
                );
                windowObj->setKeyEventBinding (GLFW_KEY_W,
                    [this](void) {
                        auto& meta             = m_cameraControllerInfo.meta;
                        auto& state            = m_cameraControllerInfo.state;
                        if (state.droneToggle == KEY_STATE_UNLOCKED)
                            meta.axialDelta    = meta.movementSensitivity;
                    }
                );
                windowObj->setKeyEventBinding (GLFW_KEY_S,
                    [this](void) {
                        auto& meta             = m_cameraControllerInfo.meta;
                        auto& state            = m_cameraControllerInfo.state;
                        if (state.droneToggle == KEY_STATE_UNLOCKED)
                            meta.axialDelta    = -meta.movementSensitivity;
                    }
                );
                windowObj->setKeyEventBinding (GLFW_KEY_MINUS,
                    [this](void) {
                        auto& meta             = m_cameraControllerInfo.meta;
                        auto& state            = m_cameraControllerInfo.state;
                        if (state.droneToggle == KEY_STATE_UNLOCKED)
                            meta.fovDelta      = meta.fovSensitivity;
                    }
                );
                windowObj->setKeyEventBinding (GLFW_KEY_EQUAL,
                    [this](void) {
                        auto& meta             = m_cameraControllerInfo.meta;
                        auto& state            = m_cameraControllerInfo.state;
                        if (state.droneToggle == KEY_STATE_UNLOCKED)
                            meta.fovDelta      = -meta.fovSensitivity;
                    }
                );
            }

        public:
            SYCameraController (void) {
                m_cameraControllerInfo = {};

                auto& logObj = m_cameraControllerInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",    __FILE__);
                logObj->updateLogConfig (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
                logObj->updateLogConfig (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
            }

            void initCameraControllerInfo (Scene::SNImpl* sceneObj,
                                           Collection::CNImpl* collectionObj) {

                if (sceneObj == nullptr || collectionObj == nullptr) {
                    LOG_ERROR (m_cameraControllerInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                       << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }

                m_cameraControllerInfo.meta.lastCursorXPos            = 0.0f;
                m_cameraControllerInfo.meta.lastCursorYPos            = 0.0f;

                m_cameraControllerInfo.meta.pitchDelta                = 0.0f;
                m_cameraControllerInfo.meta.yawDelta                  = 0.0f;
                m_cameraControllerInfo.meta.rollDelta                 = 0.0f;
                m_cameraControllerInfo.meta.lateralDelta              = 0.0f;
                m_cameraControllerInfo.meta.axialDelta                = 0.0f;
                m_cameraControllerInfo.meta.fovDelta                  = 0.0f;
                m_cameraControllerInfo.meta.minFov                    = glm::radians (  5.0f);
                m_cameraControllerInfo.meta.maxFov                    = glm::radians (120.0f);
                m_cameraControllerInfo.meta.levelDamp                 = 2.5f;
                m_cameraControllerInfo.meta.activeCamera              = {};

                m_cameraControllerInfo.state.droneToggle              = KEY_STATE_LOCKED;
                m_cameraControllerInfo.state.fineToggle               = KEY_STATE_LOCKED;
                m_cameraControllerInfo.state.levelToggle              = KEY_STATE_LOCKED;
                m_cameraControllerInfo.state.firstCursorPositionEvent = false;
                m_cameraControllerInfo.state.levelingDisabled         = true;

                m_cameraControllerInfo.resource.sceneObj              = sceneObj;
                m_cameraControllerInfo.resource.windowObj             =
                collectionObj->getCollectionTypeInstance <Renderer::VKWindow>    (
                    "DEFAULT"
                );
                m_cameraControllerInfo.resource.swapChainObj          =
                collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain> (
                    "DEFAULT"
                );

                setCameraControllerSensitivity (SENSITIVITY_TYPE_COARSE);
                setCameraControllerBindings();
            }

            ActiveCameraPC* getActiveCamera (void) {
                return &m_cameraControllerInfo.meta.activeCamera;
            }

            void update (const float frameDelta) {
                auto& meta         = m_cameraControllerInfo.meta;
                auto& state        = m_cameraControllerInfo.state;
                auto& activeCamera = meta.activeCamera;
                auto& sceneObj     = m_cameraControllerInfo.resource.sceneObj;
                auto& swapChainObj = m_cameraControllerInfo.resource.swapChainObj;

                for (auto const& entity: m_entities) {
                    auto cameraComponent    = sceneObj->getComponent <CameraComponent>    (entity);
                    auto transformComponent = sceneObj->getComponent <TransformComponent> (entity);

                    if (!cameraComponent->m_active)
                        continue;
                    /* Graphics applications and games usually keep track of a delta time variable that stores the time
                     * it took to render the last frame. We multiply the movement speed with this delta time value. The
                     * result is that when we have a large delta time in a frame, meaning that the last frame took longer
                     * than average, the velocity for that frame will also be a bit higher to balance it all out. When
                     * using this approach it does not matter if you have a very fast or slow pc, the velocity of the
                     * camera will be balanced out accordingly so each user will have the same experience
                    */
                    {   /* Rotation update */
                        transformComponent->addYaw   (meta.yawDelta   * frameDelta);
                        transformComponent->addPitch (meta.pitchDelta * frameDelta);
                        transformComponent->addRoll  (meta.rollDelta  * frameDelta);

                        meta.pitchDelta *= meta.deltaDamp;
                        meta.yawDelta   *= meta.deltaDamp;
                        meta.rollDelta  *= meta.deltaDamp;
                    }
                    {   /* Leveling */
                        if (!state.levelingDisabled) {
                            auto& orientation           = transformComponent->m_orientation;
                            glm::vec3 upVector          = transformComponent->getUpVector();
                            glm::vec3 targetUpVector    = {0.0f, 1.0f, 0.0f};
                            glm::vec3 targetAxis        = glm::cross     (upVector, targetUpVector);
                            float targetAngle           = glm::dot       (upVector, targetUpVector) * meta.levelDamp;

                            glm::quat targetOrientation = glm::angleAxis (targetAngle * frameDelta, targetAxis);
                            orientation                 = glm::normalize (targetOrientation * orientation);
                        }
                    }
                    {   /* Translation update */
                        glm::vec3 rightVector           = transformComponent->getRightVector();
                        glm::vec3 forwardVector         = transformComponent->getForwardVector();

                        transformComponent->m_position += (rightVector   * meta.lateralDelta * frameDelta) +
                                                          (forwardVector * meta.axialDelta   * frameDelta);

                        meta.lateralDelta *= meta.deltaDamp;
                        meta.axialDelta   *= meta.deltaDamp;
                    }
                    {   /* Fov update */
                        /* When the field of view becomes smaller, the scene's projected space gets smaller. This smaller
                         * space is projected over the same NDC, giving the illusion of zooming in
                        */
                        auto& fov = cameraComponent->m_fov;
                        fov      += meta.fovDelta * frameDelta;
                        fov       = std::clamp (
                            fov,
                            meta.minFov,
                            meta.maxFov
                        );
                        meta.fovDelta *= meta.deltaDamp;
                    }

                    /* Compute aspect ratio */
                    float aspectRatio             = swapChainObj->getSwapChainExtent()->width/
                                                    static_cast <float> (swapChainObj->getSwapChainExtent()->height);
                    activeCamera.position         = transformComponent->m_position;
                    activeCamera.viewMatrix       = glm::inverse (transformComponent->createModelMatrix());
                    activeCamera.projectionMatrix = cameraComponent->createProjectionMatrix (aspectRatio);
                }
            }

            ~SYCameraController (void) {
                delete m_cameraControllerInfo.resource.logObj;
            }
    };
}   // namespace SandBox