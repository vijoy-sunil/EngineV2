#pragma once
#include "../Common.h"
#include "../Collection/CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"
#include "VKInstance.h"
#include "VKWindow.h"

namespace Renderer {
    class VKSurface: public Collection::CNTypeInstanceBase {
        private:
            struct SurfaceInfo {
                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKInstance* instanceObj;
                    VKWindow* windowObj;
                    VkSurfaceKHR surface;
                } resource;
            } m_surfaceInfo;

            void createSurface (void) {
                auto& resource = m_surfaceInfo.resource;
                /* VK_KHR_surface (instance level extension) exposes a VkSurfaceKHR object that represents an abstract
                 * type of surface to present rendered images to
                */
                auto result = glfwCreateWindowSurface (*resource.instanceObj->getInstance(),
                                                        resource.windowObj->getWindow(),
                                                        nullptr,
                                                        &resource.surface);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (resource.logObj) << "[?] Surface"
                                                << " "
                                                << "[" << string_VkResult (result) << "]"
                                                << std::endl;
                    throw std::runtime_error ("[?] Surface");
                }
                LOG_INFO (resource.logObj)      << "[O] Surface"
                                                << std::endl;
            }

            void destroySurface (void) {
                auto& resource = m_surfaceInfo.resource;
                vkDestroySurfaceKHR (*resource.instanceObj->getInstance(),
                                      resource.surface,
                                      nullptr);
                LOG_INFO (resource.logObj) << "[X] Surface"
                                           << std::endl;
            }

        public:
            VKSurface (Log::LGImpl* logObj,
                       VKInstance*  instanceObj,
                       VKWindow*    windowObj) {

                m_surfaceInfo = {};

                if (logObj == nullptr) {
                    m_surfaceInfo.resource.logObj     = new Log::LGImpl();
                    m_surfaceInfo.state.logObjCreated = true;

                    m_surfaceInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
                    LOG_WARNING (m_surfaceInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                << std::endl;
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
            }

            void initSurfaceInfo (void) {
                m_surfaceInfo.resource.surface = nullptr;
            }

            VkSurfaceKHR* getSurface (void) {
                return &m_surfaceInfo.resource.surface;
            }

            void onAttach (void) override {
                createSurface();
            }

            void onDetach (void) override {
                destroySurface();
            }

            void onUpdate (void) override {
                /* Do nothing */
            }

            ~VKSurface (void) {
                if (m_surfaceInfo.state.logObjCreated)
                    delete m_surfaceInfo.resource.logObj;
            }
    };
}   // namespace Renderer