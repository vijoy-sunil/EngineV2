#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include "../Collection/CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"
#include "VKLogDevice.h"
#include "VKCmdPool.h"

namespace Renderer {
    class VKCmdBuffer: public Collection::CNTypeInstanceBase {
        private:
            struct CmdBufferInfo {
                struct Meta {
                    uint32_t buffersCount;
                    VkCommandBufferLevel bufferLevel;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKLogDevice* logDeviceObj;
                    VKCmdPool* cmdPoolObj;
                    std::vector <VkCommandBuffer> buffers;
                } resource;
            } m_cmdBufferInfo;

            void createCmdBuffers (void) {
                VkCommandBufferAllocateInfo allocInfo;
                allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.pNext              = nullptr;
                allocInfo.commandBufferCount = m_cmdBufferInfo.meta.buffersCount;
                allocInfo.level              = m_cmdBufferInfo.meta.bufferLevel;
                allocInfo.commandPool        = *m_cmdBufferInfo.resource.cmdPoolObj->getCmdPool();

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
                         VKLogDevice* logDeviceObj,
                         VKCmdPool*   cmdPoolObj) {

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

                if (logDeviceObj == nullptr || cmdPoolObj == nullptr) {
                    LOG_ERROR (m_cmdBufferInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_cmdBufferInfo.resource.logDeviceObj   = logDeviceObj;
                m_cmdBufferInfo.resource.cmdPoolObj     = cmdPoolObj;
            }

            void initCmdBufferInfo (const uint32_t cmdBuffersCount,
                                    const VkCommandBufferLevel cmdBufferLevel) {

                m_cmdBufferInfo.meta.buffersCount = cmdBuffersCount;
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

            void onUpdate (void) override {
                /* Do nothing */
            }

            ~VKCmdBuffer (void) {
                if (m_cmdBufferInfo.state.logObjCreated)
                    delete m_cmdBufferInfo.resource.logObj;
            }
    };
}   // namespace Renderer