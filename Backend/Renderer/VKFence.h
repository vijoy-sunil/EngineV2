#pragma once
#include "../Common.h"
#include "../Collection/CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"
#include "VKLogDevice.h"

namespace Renderer {
    class VKFence: public Collection::CNTypeInstanceBase {
        private:
            struct FenceInfo {
                struct Meta {
                    VkFenceCreateFlags createFlags;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKLogDevice* logDeviceObj;
                    VkFence fence;
                } resource;
            } m_fenceInfo;

            void createFence (void) {
                VkFenceCreateInfo createInfo;
                createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                createInfo.pNext = nullptr;
                createInfo.flags = m_fenceInfo.meta.createFlags;

                auto result = vkCreateFence (*m_fenceInfo.resource.logDeviceObj->getLogDevice(),
                                              &createInfo,
                                              nullptr,
                                              &m_fenceInfo.resource.fence);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_fenceInfo.resource.logObj) << "[?] Fence"
                                                            << " "
                                                            << "[" << string_VkResult (result) << "]"
                                                            << std::endl;
                    throw std::runtime_error ("[?] Fence");
                }
                LOG_INFO (m_fenceInfo.resource.logObj)      << "[O] Fence"
                                                            << std::endl;
            }

            void destroyFence (void) {
                vkDestroyFence  (*m_fenceInfo.resource.logDeviceObj->getLogDevice(),
                                  m_fenceInfo.resource.fence,
                                  nullptr);
                LOG_INFO (m_fenceInfo.resource.logObj) << "[X] Fence"
                                                       << std::endl;
            }

        public:
            VKFence (Log::LGImpl* logObj,
                     VKLogDevice* logDeviceObj) {

                m_fenceInfo = {};

                if (logObj == nullptr) {
                    m_fenceInfo.resource.logObj     = new Log::LGImpl();
                    m_fenceInfo.state.logObjCreated = true;

                    m_fenceInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
                    LOG_WARNING (m_fenceInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                              << std::endl;
                }
                else {
                    m_fenceInfo.resource.logObj     = logObj;
                    m_fenceInfo.state.logObjCreated = false;
                }

                if (logDeviceObj == nullptr) {
                    LOG_ERROR (m_fenceInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                            << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_fenceInfo.resource.logDeviceObj   = logDeviceObj;
            }

            void initFenceInfo (const VkFenceCreateFlags createFlags) {
                m_fenceInfo.meta.createFlags = createFlags;
                m_fenceInfo.resource.fence   = nullptr;
            }

            void waitForFence (void) {
                vkWaitForFences (*m_fenceInfo.resource.logDeviceObj->getLogDevice(),
                                  1,
                                  &m_fenceInfo.resource.fence,
                                  VK_TRUE,
                                  UINT64_MAX);
            }

            /* Note that, fences must be reset manually to put them back into the unsignaled state. This is because
             * fences are used to control the execution of the host, and so the host gets to decide when to reset the
             * fence. Contrast this to semaphores which are used to order work on the GPU without the host being involved
            */
            void resetFence (void) {
                vkResetFences (*m_fenceInfo.resource.logDeviceObj->getLogDevice(),
                                1,
                                &m_fenceInfo.resource.fence);
            }

            VkFence* getFence (void) {
                return &m_fenceInfo.resource.fence;
            }

            void onAttach (void) override {
                createFence();
            }

            void onDetach (void) override {
                destroyFence();
            }

            void onUpdate (void) override {
                /* Do nothing */
            }

            ~VKFence (void) {
                if (m_fenceInfo.state.logObjCreated)
                    delete m_fenceInfo.resource.logObj;
            }
    };
}   // namespace Renderer