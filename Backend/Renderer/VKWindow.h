#pragma once
#include "../Common.h"
#include "../Collection/CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"

namespace Renderer {
    class VKWindow: public Collection::CNTypeInstanceBase {
        private:
            struct KeyEventInfo {
                struct State {
                    int action;
                } state;

                struct Resource {
                    std::function <void (void)> pressBinding;
                    std::function <void (void)> releaseBinding;
                } resource;
            };

            struct WindowInfo {
                struct Meta {
                    int width;
                    int height;
                    const char* title;
                    std::unordered_map <int, KeyEventInfo> keyEventInfoPool;
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
                    std::function <void (double, double)> cursorPositionBinding;
                    std::function <void (double, double)> scrollOffsetBinding;
                } resource;
            } m_windowInfo;

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
                if (iconified) thisPtr->m_windowInfo.state.iconified = true;
                else           thisPtr->m_windowInfo.state.iconified = false;
            }

            /* Note that, as of now on Windows, the callback performs as expected where once the cursor leaves the window
             * area the callback stops firing. For OSX, the window never loses focus and therefore the callback is always
             * being called
            */
            static void cursorPositionCallback (GLFWwindow* window,
                                                const double xPos,
                                                const double yPos) {

                auto thisPtr = reinterpret_cast <VKWindow*> (glfwGetWindowUserPointer (window));
                if (thisPtr->m_windowInfo.resource.cursorPositionBinding != nullptr)
                    thisPtr->m_windowInfo.resource.cursorPositionBinding (xPos, yPos);
            }

            static void scrollOffsetCallback (GLFWwindow* window,
                                              const double xOffset,
                                              const double yOffset) {

                auto thisPtr = reinterpret_cast <VKWindow*> (glfwGetWindowUserPointer (window));
                if (thisPtr->m_windowInfo.resource.scrollOffsetBinding != nullptr)
                    thisPtr->m_windowInfo.resource.scrollOffsetBinding (xOffset, yOffset);
            }

            static void keyEventCallback (GLFWwindow* window,
                                          const int key,
                                          const int scanCode,
                                          const int action,
                                          const int mods) {

                static_cast <void> (scanCode);
                static_cast <void> (mods);

                auto thisPtr           = reinterpret_cast <VKWindow*> (glfwGetWindowUserPointer (window));
                auto& keyEventInfoPool = thisPtr->m_windowInfo.meta.keyEventInfoPool;
                /* Do not save event info if the key doesn't exist in pool */
                if (keyEventInfoPool.find (key) == keyEventInfoPool.end())
                    return;

                keyEventInfoPool[key].state.action = action;
            }

            void handleKeyEvents (void) {
                for (auto& [key, info]: m_windowInfo.meta.keyEventInfoPool) {
                    if ((info.state.action == GLFW_PRESS || info.state.action == GLFW_REPEAT) &&
                        (info.resource.pressBinding != nullptr))
                         info.resource.pressBinding();

                    if ((info.state.action == GLFW_RELEASE) &&
                        (info.resource.releaseBinding != nullptr)) {
                         info.resource.releaseBinding();
                        /* Reset action */
                        info.state.action  = GLFW_KEY_UNKNOWN;
                    }
                }
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
                toggleCursorPositionCallback (false);
                toggleScrollOffsetCallback   (false);
                toggleKeyEventCallback       (false);

                glfwDestroyWindow (m_windowInfo.resource.window);
                glfwTerminate();

                LOG_INFO (m_windowInfo.resource.logObj) << "[X] Window"
                                                        << std::endl;
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
                                 const bool resizeDisabled = false) {

                m_windowInfo.meta.width                     = width;
                m_windowInfo.meta.height                    = height;
                m_windowInfo.meta.title                     = title;
                m_windowInfo.meta.keyEventInfoPool          = {};

                m_windowInfo.state.resizeDisabled           = resizeDisabled;
                m_windowInfo.state.resized                  = false;
                m_windowInfo.state.iconified                = false;

                m_windowInfo.resource.window                = nullptr;
                m_windowInfo.resource.cursorPositionBinding = nullptr;
                m_windowInfo.resource.scrollOffsetBinding   = nullptr;
            }

            bool isWindowResized (void) {
                return m_windowInfo.state.resized;
            }

            bool isWindowIconified (void) {
                return m_windowInfo.state.iconified;
            }

            bool isWindowClosed (void) {
                return glfwWindowShouldClose (m_windowInfo.resource.window);
            }

            void toggleWindowResized (const bool val) {
                m_windowInfo.state.resized = val;
            }

            void toggleWindowClosed (const bool val) {
                glfwSetWindowShouldClose (m_windowInfo.resource.window, val);
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

            void setCursorPositionBinding (const std::function <void (double, double)> binding) {
                m_windowInfo.resource.cursorPositionBinding = binding;
            }

            void setScrollOffsetBinding (const std::function <void (double, double)> binding) {
                m_windowInfo.resource.scrollOffsetBinding = binding;
            }

            void setKeyEventBinding (const int key,
                                     const std::function <void (void)> pressBinding,
                                     const std::function <void (void)> releaseBinding = nullptr) {

                auto& keyEventInfoPool                        = m_windowInfo.meta.keyEventInfoPool;
                keyEventInfoPool[key].state.action            = GLFW_KEY_UNKNOWN;
                keyEventInfoPool[key].resource.pressBinding   = pressBinding;
                keyEventInfoPool[key].resource.releaseBinding = releaseBinding;
            }

            GLFWwindow* getWindow (void) {
                return m_windowInfo.resource.window;
            }

            void onAttach (void) override {
                createWindow();
            }

            void onDetach (void) override {
                destroyWindow();
            }

            void onUpdate (void) override {
                handleKeyEvents();
            }

            ~VKWindow (void) {
                if (m_windowInfo.state.logObjCreated)
                    delete m_windowInfo.resource.logObj;
            }
    };
}   // namespace Renderer