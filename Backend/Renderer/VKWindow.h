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
                    std::unordered_map <int, KeyEventInfo> keyToKeyEventInfoMap;
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

                auto thisPtr                = reinterpret_cast <VKWindow*> (glfwGetWindowUserPointer (window));
                auto& cursorPositionBinding = thisPtr->m_windowInfo.resource.cursorPositionBinding;
                if (cursorPositionBinding != nullptr)
                    cursorPositionBinding (xPos, yPos);
            }

            static void scrollOffsetCallback (GLFWwindow* window,
                                              const double xOffset,
                                              const double yOffset) {

                auto thisPtr              = reinterpret_cast <VKWindow*> (glfwGetWindowUserPointer (window));
                auto& scrollOffsetBinding = thisPtr->m_windowInfo.resource.scrollOffsetBinding;
                if (scrollOffsetBinding != nullptr)
                    scrollOffsetBinding (xOffset, yOffset);
            }

            static void keyEventCallback (GLFWwindow* window,
                                          const int key,
                                          const int scanCode,
                                          const int action,
                                          const int mods) {

                static_cast <void> (scanCode);
                static_cast <void> (mods);

                auto thisPtr               = reinterpret_cast <VKWindow*> (glfwGetWindowUserPointer (window));
                auto& keyToKeyEventInfoMap = thisPtr->m_windowInfo.resource.keyToKeyEventInfoMap;
                /* Do not save event info if the key doesn't exist in pool */
                if (keyToKeyEventInfoMap.find (key) == keyToKeyEventInfoMap.end())
                    return;

                keyToKeyEventInfoMap[key].state.action = action;
            }

            void handleKeyEvents (void) {
                for (auto& [key, info]: m_windowInfo.resource.keyToKeyEventInfoMap) {
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
                auto& meta     = m_windowInfo.meta;
                auto& resource = m_windowInfo.resource;

                glfwInit();
                glfwWindowHint (GLFW_CLIENT_API, GLFW_NO_API);
                if (m_windowInfo.state.resizeDisabled)
                    glfwWindowHint (GLFW_RESIZABLE, GLFW_FALSE);

                resource.window = glfwCreateWindow (meta.width,
                                                    meta.height,
                                                    meta.title,
                                                    nullptr,
                                                    nullptr);

                glfwSetWindowUserPointer       (resource.window, this);
                glfwSetFramebufferSizeCallback (resource.window, windowResizeCallback);
                glfwSetWindowIconifyCallback   (resource.window, windowIconifyCallback);

                LOG_INFO (resource.logObj) << "[O] Window"
                                           << std::endl;
            }

            void destroyWindow (void) {
                auto& resource = m_windowInfo.resource;
                toggleCursorPositionCallback (false);
                toggleScrollOffsetCallback   (false);
                toggleKeyEventCallback       (false);

                glfwDestroyWindow            (resource.window);
                glfwTerminate();

                LOG_INFO (resource.logObj) << "[X] Window"
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

                auto& meta                     = m_windowInfo.meta;
                auto& state                    = m_windowInfo.state;
                auto& resource                 = m_windowInfo.resource;

                meta.width                     = width;
                meta.height                    = height;
                meta.title                     = title;

                state.resizeDisabled           = resizeDisabled;
                state.resized                  = false;
                state.iconified                = false;

                resource.window                = nullptr;
                resource.cursorPositionBinding = nullptr;
                resource.scrollOffsetBinding   = nullptr;
                resource.keyToKeyEventInfoMap  = {};
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
                auto& window = m_windowInfo.resource.window;
                if (val) {
                    glfwSetCursorPosCallback (window, cursorPositionCallback);
                    glfwSetInputMode         (window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
                else {
                    glfwSetCursorPosCallback (window, nullptr);
                    glfwSetInputMode         (window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
            }

            void toggleScrollOffsetCallback (const bool val) {
                auto& window = m_windowInfo.resource.window;
                if (val) {
                    glfwSetScrollCallback (window, scrollOffsetCallback);
                    glfwSetInputMode      (window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
                else {
                    glfwSetScrollCallback (window, nullptr);
                    glfwSetInputMode      (window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
            }

            void toggleKeyEventCallback (const bool val) {
                auto& window = m_windowInfo.resource.window;
                if (val)
                    glfwSetKeyCallback (window, keyEventCallback);
                else
                    glfwSetKeyCallback (window, nullptr);
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

                auto& keyToKeyEventInfoMap                        = m_windowInfo.resource.keyToKeyEventInfoMap;
                keyToKeyEventInfoMap[key].state.action            = GLFW_KEY_UNKNOWN;
                keyToKeyEventInfoMap[key].resource.pressBinding   = pressBinding;
                keyToKeyEventInfoMap[key].resource.releaseBinding = releaseBinding;
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