#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vk_enum_string_helper.h>
#include "../Backend/Collection/CNTypeInstanceBase.h"
#include "../Backend/Log/LGImpl.h"
#include "VKLogDevice.h"

namespace Renderer {
    class VKCmdPool: public Collection::CNTypeInstanceBase {
        private:
            struct CmdPoolInfo {
                struct Meta {
                    VkCommandPoolCreateFlags createFlags;
                    uint32_t queueFamilyIdx;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKLogDevice* logDeviceObj;
                    VkCommandPool pool;
                } resource;
            } m_cmdPoolInfo;

        public:
            VKCmdPool (Log::LGImpl* logObj,
                       VKLogDevice* logDeviceObj) {

                m_cmdPoolInfo = {};

                if (logObj == nullptr) {
                    m_cmdPoolInfo.resource.logObj     = new Log::LGImpl();
                    m_cmdPoolInfo.state.logObjCreated = true;

                    m_cmdPoolInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
                    LOG_WARNING (m_cmdPoolInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                << std::endl;
                }
                else {
                    m_cmdPoolInfo.resource.logObj     = logObj;
                    m_cmdPoolInfo.state.logObjCreated = false;
                }

                if (logDeviceObj == nullptr) {
                    LOG_ERROR (m_cmdPoolInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                              << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_cmdPoolInfo.resource.logDeviceObj   = logDeviceObj;
            }

            void initCmdPoolInfo (const VkCommandPoolCreateFlags createFlags,
                                  const uint32_t queueFamilyIdx) {

                m_cmdPoolInfo.meta.createFlags    = createFlags;
                m_cmdPoolInfo.meta.queueFamilyIdx = queueFamilyIdx;
                m_cmdPoolInfo.resource.pool       = nullptr;
            }

            VkCommandPool* getCmdPool (void) {
                return &m_cmdPoolInfo.resource.pool;
            }

            void createCmdPool (void) {
                VkCommandPoolCreateInfo createInfo;
                createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                createInfo.pNext            = nullptr;
                createInfo.flags            = m_cmdPoolInfo.meta.createFlags;
                createInfo.queueFamilyIndex = m_cmdPoolInfo.meta.queueFamilyIdx;

                auto result = vkCreateCommandPool (*m_cmdPoolInfo.resource.logDeviceObj->getLogDevice(),
                                                    &createInfo,
                                                    nullptr,
                                                    &m_cmdPoolInfo.resource.pool);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_cmdPoolInfo.resource.logObj) << "[?] Cmd pool"
                                                              << " "
                                                              << "[" << string_VkResult (result) << "]"
                                                              << std::endl;
                    throw std::runtime_error ("[?] Cmd pool");
                }
                LOG_INFO (m_cmdPoolInfo.resource.logObj)      << "[O] Cmd pool"
                                                              << std::endl;
            }

            void destroyCmdPool (void) {
                vkDestroyCommandPool (*m_cmdPoolInfo.resource.logDeviceObj->getLogDevice(),
                                       m_cmdPoolInfo.resource.pool,
                                       nullptr);
                LOG_INFO (m_cmdPoolInfo.resource.logObj) << "[X] Cmd pool"
                                                         << std::endl;
            }

            void onAttach (void) override {
                createCmdPool();
            }

            void onDetach (void) override {
                destroyCmdPool();
            }

            void onUpdate (const float frameDelta) override {
                static_cast <void> (frameDelta);
                /* Do nothing */
            }

            ~VKCmdPool (void) {
                if (m_cmdPoolInfo.state.logObjCreated)
                    delete m_cmdPoolInfo.resource.logObj;
            }
    };
}   // namespace Renderer