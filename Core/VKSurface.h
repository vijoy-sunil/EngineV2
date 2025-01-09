#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vk_enum_string_helper.h>
#include "../Backend/Layer/LYInstanceBase.h"
#include "../Backend/Log/LGImpl.h"
#include "VKInstance.h"
#include "VKWindow.h"

namespace Core {
    class VKSurface: public Layer::LYInstanceBase {
        private:
            struct SurfaceInfo {
                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKInstance*  instanceObj;
                    VKWindow*    windowObj;
                    VkSurfaceKHR surface;
                } resource;
            } m_surfaceInfo;

        public:
            VKSurface (Log::LGImpl* logObj,
                       VKInstance*  instanceObj,
                       VKWindow*    windowObj) {

                if (logObj == nullptr) {
                    m_surfaceInfo.resource.logObj     = new Log::LGImpl();
                    LOG_WARNING (m_surfaceInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                << std::endl;
                    m_surfaceInfo.state.logObjCreated = true;
                }
                else {
                    m_surfaceInfo.resource.logObj     = logObj;
                    m_surfaceInfo.state.logObjCreated = false;
                }

                if (instanceObj == nullptr || windowObj == nullptr) {
                    LOG_ERROR (m_surfaceInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                              << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }

                m_surfaceInfo.resource.instanceObj    = instanceObj;
                m_surfaceInfo.resource.windowObj      = windowObj;
                m_surfaceInfo.resource.surface        = nullptr;
            }

            VkSurfaceKHR* getSurface (void) {
                return &m_surfaceInfo.resource.surface;
            }

            void createSurface (void) {
                /* VK_KHR_surface (instance level extension) exposes a VkSurfaceKHR object that represents an abstract
                 * type of surface to present rendered images to
                */
                auto result = glfwCreateWindowSurface (*m_surfaceInfo.resource.instanceObj->getInstance(),
                                                        m_surfaceInfo.resource.windowObj->getWindow(),
                                                        nullptr,
                                                        &m_surfaceInfo.resource.surface);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_surfaceInfo.resource.logObj) << "[?] Surface"
                                                              << " "
                                                              << "[" << string_VkResult (result) << "]"
                                                              << std::endl;
                    throw std::runtime_error ("[?] Surface");
                }
                LOG_INFO (m_surfaceInfo.resource.logObj) << "[O] Surface"
                                                         << std::endl;
            }

            void destroySurface (void) {
                vkDestroySurfaceKHR (*m_surfaceInfo.resource.instanceObj->getInstance(),
                                      m_surfaceInfo.resource.surface,
                                      nullptr);
                LOG_INFO (m_surfaceInfo.resource.logObj) << "[X] Surface"
                                                         << std::endl;

                if (m_surfaceInfo.state.logObjCreated)
                    delete m_surfaceInfo.resource.logObj;
            }
    };
}   // namespace Core