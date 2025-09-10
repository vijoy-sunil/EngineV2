#pragma once
#include "../Common.h"
#include "../Collection/CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"
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

            void createCmdPool (void) {
                auto& meta                  = m_cmdPoolInfo.meta;
                auto& resource              = m_cmdPoolInfo.resource;

                VkCommandPoolCreateInfo createInfo;
                createInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                createInfo.pNext            = nullptr;
                createInfo.flags            = meta.createFlags;
                createInfo.queueFamilyIndex = meta.queueFamilyIdx;

                auto result = vkCreateCommandPool (*resource.logDeviceObj->getLogDevice(),
                                                    &createInfo,
                                                    nullptr,
                                                    &resource.pool);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (resource.logObj) << "[?] Cmd pool"
                                                << " "
                                                << "[" << string_VkResult (result) << "]"
                                                << std::endl;
                    throw std::runtime_error ("[?] Cmd pool");
                }
                LOG_INFO (resource.logObj)      << "[O] Cmd pool"
                                                << std::endl;
            }

            void destroyCmdPool (void) {
                auto& resource = m_cmdPoolInfo.resource;
                vkDestroyCommandPool (*resource.logDeviceObj->getLogDevice(),
                                       resource.pool,
                                       nullptr);
                LOG_INFO (resource.logObj) << "[X] Cmd pool"
                                           << std::endl;
            }

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

                auto& meta                  = m_cmdPoolInfo.meta;
                meta.createFlags            = createFlags;
                meta.queueFamilyIdx         = queueFamilyIdx;
                m_cmdPoolInfo.resource.pool = nullptr;
            }

            VkCommandPool* getCmdPool (void) {
                return &m_cmdPoolInfo.resource.pool;
            }

            void onAttach (void) override {
                createCmdPool();
            }

            void onDetach (void) override {
                destroyCmdPool();
            }

            void onUpdate (void) override {
                /* Do nothing */
            }

            ~VKCmdPool (void) {
                if (m_cmdPoolInfo.state.logObjCreated)
                    delete m_cmdPoolInfo.resource.logObj;
            }
    };
}   // namespace Renderer