#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <functional>
#include "../Backend/Layer/LYInstanceBase.h"
#include "../Backend/Log/LGImpl.h"

namespace Renderer {
    class VKWindow: public Layer::LYInstanceBase {
        private:
            struct WindowInfo {
                struct Meta {
                    int width;
                    int height;
                    const char* title;
                } meta;

                struct State {
                    bool resizeDisabled;
                    bool resized;
                    bool iconified;
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    GLFWwindow* window;
                    /* Bindings */
                    std::function <void (double, double)> runCursorPosition;
                    std::function <void (double, double)> runScrollOffset;
                } resource;
            } m_windowInfo;

            struct KeyEventInfo {
                struct State {
                    bool pressed;
                } state;

                struct Resource {
                    std::function <void (float)> run;
                } resource;
            };
            std::unordered_map <int, KeyEventInfo> m_keyEventInfoPool;

            /* Callbacks */
            static void windowResizeCallback (GLFWwindow* window,
                                              const int width,
                                              const int height) {

                /* Suppress unused parameter warning */
                static_cast <void> (width);
                static_cast <void> (height);

                auto thisPtr = reinterpret_cast <VKWindow*> (glfwGetWindowUserPointer (window));
                /* Although many drivers and platforms trigger VK_ERROR_OUT_OF_DATE_KHR automatically after a window
                 * resize, it is not guaranteed to happen. That's why we'll add some extra code to handle resizes
                 * explicitly using this boolean
                */
                thisPtr->toggleWindowResized (true);
            }

            static void windowIconifyCallback (GLFWwindow* window,
                                               const int iconified) {

                auto thisPtr = reinterpret_cast <VKWindow*> (glfwGetWindowUserPointer (window));
                if (iconified)  thisPtr->m_windowInfo.state.iconified = true;
                else            thisPtr->m_windowInfo.state.iconified = false;
            }

            /* Note that, as of now on Windows, the callback performs as expected where once the mouse leaves the window
             * area the callback stops firing. For OSX, the window never loses focus and therefore the cursor callback
             * is always being called
            */
            static void cursorPositionCallback (GLFWwindow* window,
                                                const double xPos,
                                                const double yPos) {

                auto thisPtr = reinterpret_cast <VKWindow*> (glfwGetWindowUserPointer (window));
                if (thisPtr->m_windowInfo.resource.runCursorPosition != nullptr)
                    thisPtr->m_windowInfo.resource.runCursorPosition (xPos, yPos);
            }

            static void scrollOffsetCallback (GLFWwindow* window,
                                              const double xOffset,
                                              const double yOffset) {

                auto thisPtr = reinterpret_cast <VKWindow*> (glfwGetWindowUserPointer (window));
                if (thisPtr->m_windowInfo.resource.runScrollOffset != nullptr)
                    thisPtr->m_windowInfo.resource.runScrollOffset (xOffset, yOffset);
            }

            static void keyEventCallback (GLFWwindow* window,
                                          const int key,
                                          const int scanCode,
                                          const int action,
                                          const int mods) {

                static_cast <void>  (scanCode);
                static_cast <void>  (mods);

                auto thisPtr = reinterpret_cast <VKWindow*> (glfwGetWindowUserPointer (window));
                /* Do not save event info if the key doesn't exist in pool */
                if (thisPtr->m_keyEventInfoPool.find (key) == thisPtr->m_keyEventInfoPool.end())
                    return;

                bool pressed = thisPtr->m_keyEventInfoPool[key].state.pressed;
                if (action == GLFW_PRESS && !pressed)
                    thisPtr->m_keyEventInfoPool[key].state.pressed = true;
                if (action == GLFW_RELEASE)
                    thisPtr->m_keyEventInfoPool[key].state.pressed = false;
            }

        public:
            VKWindow (Log::LGImpl* logObj) {
                m_windowInfo = {};

                if (logObj == nullptr) {
                    m_windowInfo.resource.logObj     = new Log::LGImpl();
                    m_windowInfo.state.logObjCreated = true;

                    m_windowInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
                    LOG_WARNING (m_windowInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                               << std::endl;
                }
                else {
                    m_windowInfo.resource.logObj     = logObj;
                    m_windowInfo.state.logObjCreated = false;
                }
            }

            void initWindowInfo (const int width,
                                 const int height,
                                 const char* title,
                                 const bool resizeDisabled = false,
                                 const std::function <void (double, double)> runCursorPosition = nullptr,
                                 const std::function <void (double, double)> runScrollOffset   = nullptr) {

                m_windowInfo.meta.width                 = width;
                m_windowInfo.meta.height                = height;
                m_windowInfo.meta.title                 = title;

                m_windowInfo.state.resizeDisabled       = resizeDisabled;
                m_windowInfo.state.resized              = false;
                m_windowInfo.state.iconified            = false;

                m_windowInfo.resource.window            = nullptr;
                m_windowInfo.resource.runCursorPosition = runCursorPosition;
                m_windowInfo.resource.runScrollOffset   = runScrollOffset;
            }

            bool isWindowResized (void) {
                return m_windowInfo.state.resized;
            }

            bool isWindowIconified (void) {
                return m_windowInfo.state.iconified;
            }

            void toggleWindowResized (const bool val) {
                m_windowInfo.state.resized = val;
            }

            void toggleCursorPositionCallback (const bool val) {
                if (val) {
                    glfwSetCursorPosCallback (m_windowInfo.resource.window, cursorPositionCallback);
                    glfwSetInputMode         (m_windowInfo.resource.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
                else {
                    glfwSetCursorPosCallback (m_windowInfo.resource.window, nullptr);
                    glfwSetInputMode         (m_windowInfo.resource.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
            }

            void toggleScrollOffsetCallback (const bool val) {
                if (val) {
                    glfwSetScrollCallback (m_windowInfo.resource.window, scrollOffsetCallback);
                    glfwSetInputMode      (m_windowInfo.resource.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
                else {
                    glfwSetScrollCallback (m_windowInfo.resource.window, nullptr);
                    glfwSetInputMode      (m_windowInfo.resource.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
            }

            void toggleKeyEventCallback (const bool val) {
                if (val)
                    glfwSetKeyCallback (m_windowInfo.resource.window, keyEventCallback);
                else
                    glfwSetKeyCallback (m_windowInfo.resource.window, nullptr);
            }

            void setKeyEventBinding (const int key, const std::function <void (float)> binding) {
                m_keyEventInfoPool[key].state.pressed = false;
                m_keyEventInfoPool[key].resource.run  = binding;
            }

            void handleKeyEvents (const float frameDelta) {
                for (auto const& [key, info]: m_keyEventInfoPool) {
                    if ((info.state.pressed) && (info.resource.run != nullptr))
                        info.resource.run (frameDelta);
                }
            }

            GLFWwindow* getWindow (void) {
                return m_windowInfo.resource.window;
            }

            void createWindow (void) {
                glfwInit();
                glfwWindowHint (GLFW_CLIENT_API, GLFW_NO_API);
                if (m_windowInfo.state.resizeDisabled)
                    glfwWindowHint (GLFW_RESIZABLE, GLFW_FALSE);

                m_windowInfo.resource.window = glfwCreateWindow (m_windowInfo.meta.width,
                                                                 m_windowInfo.meta.height,
                                                                 m_windowInfo.meta.title,
                                                                 nullptr,
                                                                 nullptr);

                glfwSetWindowUserPointer       (m_windowInfo.resource.window, this);
                glfwSetFramebufferSizeCallback (m_windowInfo.resource.window, windowResizeCallback);
                glfwSetWindowIconifyCallback   (m_windowInfo.resource.window, windowIconifyCallback);

                LOG_INFO (m_windowInfo.resource.logObj) << "[O] Window"
                                                        << std::endl;
            }

            void destroyWindow (void) {
                /* Disable all callbacks */
                toggleCursorPositionCallback (false);
                toggleScrollOffsetCallback   (false);
                toggleKeyEventCallback       (false);

                glfwDestroyWindow (m_windowInfo.resource.window);
                glfwTerminate();

                LOG_INFO (m_windowInfo.resource.logObj) << "[X] Window"
                                                        << std::endl;
            }

            ~VKWindow (void) {
                if (m_windowInfo.state.logObjCreated)
                    delete m_windowInfo.resource.logObj;
            }
    };
}   // namespace Renderer