#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include "../Backend/Collection/CNTypeInstanceBase.h"
#include "../Backend/Log/LGImpl.h"
#include "VKLogDevice.h"

namespace Renderer {
    class VKCmdBuffer: public Collection::CNTypeInstanceBase {
        private:
            struct CmdBufferInfo {
                struct Meta {
                    uint32_t buffersCount;
                    VkCommandPool pool;
                    VkCommandBufferLevel bufferLevel;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKLogDevice* logDeviceObj;
                    std::vector <VkCommandBuffer> buffers;
                } resource;
            } m_cmdBufferInfo;

            void createCmdBuffers (void) {
                VkCommandBufferAllocateInfo allocInfo;
                allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.pNext              = nullptr;
                allocInfo.commandBufferCount = m_cmdBufferInfo.meta.buffersCount;
                allocInfo.commandPool        = m_cmdBufferInfo.meta.pool;
                allocInfo.level              = m_cmdBufferInfo.meta.bufferLevel;

                m_cmdBufferInfo.resource.buffers.resize (m_cmdBufferInfo.meta.buffersCount);
                auto result = vkAllocateCommandBuffers  (*m_cmdBufferInfo.resource.logDeviceObj->getLogDevice(),
                                                          &allocInfo,
                                                          m_cmdBufferInfo.resource.buffers.data());
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_cmdBufferInfo.resource.logObj) << "[?] Cmd buffer(s)"
                                                                << " "
                                                                << "[" << string_VkResult (result) << "]"
                                                                << std::endl;
                    throw std::runtime_error ("[?] Cmd buffer(s)");
                }
                LOG_INFO (m_cmdBufferInfo.resource.logObj)      << "[O] Cmd buffer(s)"
                                                                << std::endl;
            }

            void destroyCmdBuffers (void) {
                LOG_INFO (m_cmdBufferInfo.resource.logObj) << "[X] Cmd buffer(s)"
                                                           << std::endl;
            }

        public:
            VKCmdBuffer (Log::LGImpl* logObj,
                         VKLogDevice* logDeviceObj) {

                m_cmdBufferInfo = {};

                if (logObj == nullptr) {
                    m_cmdBufferInfo.resource.logObj     = new Log::LGImpl();
                    m_cmdBufferInfo.state.logObjCreated = true;

                    m_cmdBufferInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
                    LOG_WARNING (m_cmdBufferInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                  << std::endl;
                }
                else {
                    m_cmdBufferInfo.resource.logObj     = logObj;
                    m_cmdBufferInfo.state.logObjCreated = false;
                }

                if (logDeviceObj == nullptr) {
                    LOG_ERROR (m_cmdBufferInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_cmdBufferInfo.resource.logDeviceObj   = logDeviceObj;
            }

            void initCmdBufferInfo (const uint32_t cmdBuffersCount,
                                    const VkCommandPool cmdPool,
                                    const VkCommandBufferLevel cmdBufferLevel) {

                m_cmdBufferInfo.meta.buffersCount = cmdBuffersCount;
                m_cmdBufferInfo.meta.pool         = cmdPool;
                m_cmdBufferInfo.meta.bufferLevel  = cmdBufferLevel;
                m_cmdBufferInfo.resource.buffers  = {};
            }

            std::vector <VkCommandBuffer>& getCmdBuffers (void) {
                return m_cmdBufferInfo.resource.buffers;
            }

            void onAttach (void) override {
                createCmdBuffers();
            }

            void onDetach (void) override {
                destroyCmdBuffers();
            }

            void onUpdate (const float frameDelta) override {
                static_cast <void> (frameDelta);
                /* Do nothing */
            }

            ~VKCmdBuffer (void) {
                if (m_cmdBufferInfo.state.logObjCreated)
                    delete m_cmdBufferInfo.resource.logObj;
            }
    };
}   // namespace Renderer