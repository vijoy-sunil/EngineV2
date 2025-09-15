#pragma once
#include "../../../Backend/Common.h"
#include "../../../Backend/Scene/SNSystemBase.h"
#include "../../../Backend/Scene/SNImpl.h"
#include "../../../Backend/Collection/CNImpl.h"
#include "../../../Backend/Log/LGImpl.h"
#include "../../../Backend/Renderer/VKWindow.h"
#include "../../../Backend/Renderer/VKSwapChain.h"
#include "../../../Backend/Renderer/VKGui.h"
#include "../../../Backend/Scene/SNType.h"
#include "SYControllerHelper.h"
#include "../SYConfig.h"
#include "../../SBComponentType.h"
#include "../../SBRendererType.h"

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
                    Renderer::VKGui* guiObj;
                } resource;
            } m_cameraControllerInfo;

            void updateCameraControllerSensitivity (const e_sensitivityType sensitivityType) {
                auto& meta                   = m_cameraControllerInfo.meta;
                auto& fineSensitivity        = g_systemConfig.camera.fineSensitivity;
                auto& coarseSensitivity      = g_systemConfig.camera.coarseSensitivity;

                if (sensitivityType         == SENSITIVITY_TYPE_FINE) {
                    meta.cursorSensitivity   = fineSensitivity.cursor;
                    meta.scrollSensitivity   = fineSensitivity.scroll;
                    meta.movementSensitivity = fineSensitivity.movement;
                    meta.fovSensitivity      = fineSensitivity.fov;
                    meta.deltaDamp           = fineSensitivity.deltaDamp;
                }
                else {
                    meta.cursorSensitivity   = coarseSensitivity.cursor;
                    meta.scrollSensitivity   = coarseSensitivity.scroll;
                    meta.movementSensitivity = coarseSensitivity.movement;
                    meta.fovSensitivity      = coarseSensitivity.fov;
                    meta.deltaDamp           = coarseSensitivity.deltaDamp;
                }
            }

            void setCameraControllerBindings (void) {
                auto& windowObj = m_cameraControllerInfo.resource.windowObj;

                windowObj->setKeyEventBinding (GLFW_KEY_GRAVE_ACCENT,
                    [this](void) {
                        auto& droneToggle = m_cameraControllerInfo.state.droneToggle;
                        if (droneToggle  == KEY_STATE_LOCKED)
                            droneToggle   = KEY_STATE_PENDING_UNLOCK;

                        if (droneToggle  == KEY_STATE_UNLOCKED)
                            droneToggle   = KEY_STATE_PENDING_LOCK;
                    },
                    [this](void) {
                        auto& state            = m_cameraControllerInfo.state;
                        auto& resource         = m_cameraControllerInfo.resource;

                        if (state.droneToggle == KEY_STATE_PENDING_UNLOCK) {
                            state.droneToggle  = KEY_STATE_UNLOCKED;

                            state.firstCursorPositionEvent = true;
                            /* Note that, the application callbacks are initialized before initializing the imgui backend.
                             * However, there may be cases where callbacks would need to be initialized after, in such
                             * cases use imgui _RestoreCallbacks and _InstallCallbacks() methods to reinstall callbacks
                            */
                            ImGui_ImplGlfw_RestoreCallbacks                  (resource.windowObj->getWindow());
                            resource.windowObj->toggleCursorPositionCallback (true);
                            resource.windowObj->toggleScrollOffsetCallback   (true);
                            ImGui_ImplGlfw_InstallCallbacks                  (resource.windowObj->getWindow());
                            resource.guiObj->toggleCursorInput               (false);
                        }

                        if (state.droneToggle == KEY_STATE_PENDING_LOCK) {
                            state.droneToggle  = KEY_STATE_LOCKED;

                            ImGui_ImplGlfw_RestoreCallbacks                  (resource.windowObj->getWindow());
                            resource.windowObj->toggleCursorPositionCallback (false);
                            resource.windowObj->toggleScrollOffsetCallback   (false);
                            ImGui_ImplGlfw_InstallCallbacks                  (resource.windowObj->getWindow());
                            resource.guiObj->toggleCursorInput               (true);
                        }
                    }
                );
                windowObj->setKeyEventBinding (GLFW_KEY_SLASH,
                    [this](void) {
                        auto& fineToggle = m_cameraControllerInfo.state.fineToggle;
                        if (fineToggle  == KEY_STATE_LOCKED) {
                            fineToggle   = KEY_STATE_UNLOCKED;
                            updateCameraControllerSensitivity (SENSITIVITY_TYPE_FINE);
                        }
                    },
                    [this](void) {
                        auto& fineToggle = m_cameraControllerInfo.state.fineToggle;
                        if (fineToggle  == KEY_STATE_UNLOCKED) {
                            fineToggle   = KEY_STATE_LOCKED;
                            updateCameraControllerSensitivity (SENSITIVITY_TYPE_COARSE);
                        }
                    }
                );
                windowObj->setKeyEventBinding (GLFW_KEY_COMMA,
                    [this](void) {
                        auto& state                = m_cameraControllerInfo.state;
                        if (state.levelToggle     == KEY_STATE_LOCKED) {
                            state.levelToggle      = KEY_STATE_UNLOCKED;
                            state.levelingDisabled = false;
                        }
                    },
                    [this](void) {
                        auto& state                = m_cameraControllerInfo.state;
                        if (state.levelToggle     == KEY_STATE_UNLOCKED) {
                            state.levelToggle      = KEY_STATE_LOCKED;
                            state.levelingDisabled = true;
                        }
                    }
                );
                windowObj->setCursorPositionBinding (
                    [this](const double xPos, const double yPos) {
                        auto& meta                     = m_cameraControllerInfo.meta;
                        auto& firstCursorPositionEvent = m_cameraControllerInfo.state.firstCursorPositionEvent;

                        if (firstCursorPositionEvent) {
                            meta.lastCursorXPos        = static_cast <float> (xPos);
                            meta.lastCursorYPos        = static_cast <float> (yPos);
                            firstCursorPositionEvent   = false;
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
                        auto& meta                                    = m_cameraControllerInfo.meta;
                        if (m_cameraControllerInfo.state.droneToggle == KEY_STATE_UNLOCKED)
                            meta.lateralDelta                         = meta.movementSensitivity;
                    }
                );
                windowObj->setKeyEventBinding (GLFW_KEY_A,
                    [this](void) {
                        auto& meta                                    = m_cameraControllerInfo.meta;
                        if (m_cameraControllerInfo.state.droneToggle == KEY_STATE_UNLOCKED)
                            meta.lateralDelta                         = -meta.movementSensitivity;
                    }
                );
                windowObj->setKeyEventBinding (GLFW_KEY_W,
                    [this](void) {
                        auto& meta                                    = m_cameraControllerInfo.meta;
                        if (m_cameraControllerInfo.state.droneToggle == KEY_STATE_UNLOCKED)
                            meta.axialDelta                           = meta.movementSensitivity;
                    }
                );
                windowObj->setKeyEventBinding (GLFW_KEY_S,
                    [this](void) {
                        auto& meta                                    = m_cameraControllerInfo.meta;
                        if (m_cameraControllerInfo.state.droneToggle == KEY_STATE_UNLOCKED)
                            meta.axialDelta                           = -meta.movementSensitivity;
                    }
                );
                windowObj->setKeyEventBinding (GLFW_KEY_MINUS,
                    [this](void) {
                        auto& meta                                    = m_cameraControllerInfo.meta;
                        if (m_cameraControllerInfo.state.droneToggle == KEY_STATE_UNLOCKED)
                            meta.fovDelta                             = meta.fovSensitivity;
                    }
                );
                windowObj->setKeyEventBinding (GLFW_KEY_EQUAL,
                    [this](void) {
                        auto& meta                                    = m_cameraControllerInfo.meta;
                        if (m_cameraControllerInfo.state.droneToggle == KEY_STATE_UNLOCKED)
                            meta.fovDelta                             = -meta.fovSensitivity;
                    }
                );
            }

        public:
            SYCameraController (void) {
                m_cameraControllerInfo = {};

                auto& logObj = m_cameraControllerInfo.resource.logObj;
                logObj       = new Log::LGImpl();
                logObj->initLogInfo     ("Build/Log/SandBox",     __FILE__);
                logObj->updateLogConfig (Log::LEVEL_TYPE_INFO,    Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_WARNING, Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
                logObj->updateLogConfig (Log::LEVEL_TYPE_ERROR,   Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
            }

            void initCameraControllerInfo (Scene::SNImpl* sceneObj,
                                           Collection::CNImpl* collectionObj) {

                auto& meta                     = m_cameraControllerInfo.meta;
                auto& state                    = m_cameraControllerInfo.state;
                auto& resource                 = m_cameraControllerInfo.resource;

                meta.lastCursorXPos            = 0.0f;
                meta.lastCursorYPos            = 0.0f;
                meta.pitchDelta                = 0.0f;
                meta.yawDelta                  = 0.0f;
                meta.rollDelta                 = 0.0f;
                meta.lateralDelta              = 0.0f;
                meta.axialDelta                = 0.0f;
                meta.fovDelta                  = 0.0f;
                meta.minFov                    = glm::radians (  5.0f);
                meta.maxFov                    = glm::radians (120.0f);
                meta.activeCamera              = {};

                state.droneToggle              = KEY_STATE_LOCKED;
                state.fineToggle               = KEY_STATE_LOCKED;
                state.levelToggle              = KEY_STATE_LOCKED;
                state.firstCursorPositionEvent = false;
                state.levelingDisabled         = true;

                if (sceneObj == nullptr || collectionObj == nullptr) {
                    LOG_ERROR (resource.logObj) << NULL_DEPOBJ_MSG
                                                << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                resource.sceneObj              = sceneObj;
                resource.windowObj             = collectionObj->getCollectionTypeInstance <Renderer::VKWindow>    (
                    "CORE"
                );
                resource.swapChainObj          = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain> (
                    "CORE"
                );
                resource.guiObj                = collectionObj->getCollectionTypeInstance <Renderer::VKGui>       (
                    "DRAW_OPS"
                );
                updateCameraControllerSensitivity (SENSITIVITY_TYPE_COARSE);
                setCameraControllerBindings();
            }

            ActiveCameraPC* getActiveCamera (void) {
                return &m_cameraControllerInfo.meta.activeCamera;
            }

            void update (const float frameDelta,
                         const Scene::Entity activeCameraEntity) {

                auto& meta     = m_cameraControllerInfo.meta;
                auto& resource = m_cameraControllerInfo.resource;

                for (auto const& entity: m_entities) {
                    if (entity != activeCameraEntity)
                        continue;

                    auto cameraComponent    = resource.sceneObj->getComponent <CameraComponent>    (entity);
                    auto transformComponent = resource.sceneObj->getComponent <TransformComponent> (entity);
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
                        if (!m_cameraControllerInfo.state.levelingDisabled) {
                            auto& orientation           = transformComponent->m_orientation;
                            glm::vec3 upVector          = transformComponent->getUpVector();
                            glm::vec3 targetUpVector    = {0.0f, 1.0f, 0.0f};
                            glm::vec3 targetAxis        = glm::cross     (upVector, targetUpVector);
                            float targetAngle           = glm::dot       (upVector, targetUpVector) *
                                                          g_systemConfig.camera.levelDamp;

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
                    auto imageExtent                   = resource.swapChainObj->getSwapChainExtent();
                    float aspectRatio                  = imageExtent->width / static_cast <float> (imageExtent->height);
                    meta.activeCamera.position         = transformComponent->m_position;
                    meta.activeCamera.viewMatrix       = glm::inverse (transformComponent->createModelMatrix (true));
                    meta.activeCamera.projectionMatrix = cameraComponent->createProjectionMatrix (aspectRatio);
                }
            }

            ~SYCameraController (void) {
                delete m_cameraControllerInfo.resource.logObj;
            }
    };
}   // namespace SandBox