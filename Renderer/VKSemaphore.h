#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vk_enum_string_helper.h>
#include "../Backend/Collection/CNTypeInstanceBase.h"
#include "../Backend/Log/LGImpl.h"
#include "VKLogDevice.h"

namespace Renderer {
    class VKSemaphore: public Collection::CNTypeInstanceBase {
        private:
            struct SemaphoreInfo {
                struct Meta {
                    VkSemaphoreCreateFlags createFlags;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKLogDevice* logDeviceObj;
                    VkSemaphore semaphore;
                } resource;
            } m_semaphoreInfo;

        public:
            VKSemaphore (Log::LGImpl* logObj,
                         VKLogDevice* logDeviceObj) {

                m_semaphoreInfo = {};

                if (logObj == nullptr) {
                    m_semaphoreInfo.resource.logObj     = new Log::LGImpl();
                    m_semaphoreInfo.state.logObjCreated = true;

                    m_semaphoreInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
                    LOG_WARNING (m_semaphoreInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                  << std::endl;
                }
                else {
                    m_semaphoreInfo.resource.logObj     = logObj;
                    m_semaphoreInfo.state.logObjCreated = false;
                }

                if (logDeviceObj == nullptr) {
                    LOG_ERROR (m_semaphoreInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_semaphoreInfo.resource.logDeviceObj   = logDeviceObj;
            }

            void initSemaphoreInfo (const VkSemaphoreCreateFlags createFlags) {
                m_semaphoreInfo.meta.createFlags   = createFlags;
                m_semaphoreInfo.resource.semaphore = nullptr;
            }

            VkSemaphore* getSemaphore (void) {
                return &m_semaphoreInfo.resource.semaphore;
            }

            /* A semaphore is used to add order between queue operations. Queue operations refer to the work we submit
             * to a queue, either in a command buffer or from within a function. Semaphores are used both to order work
             * inside the same queue and between different queues
             *
             * The way we use a semaphore to order queue operations is by providing the same semaphore as a 'signal'
             * semaphore in one queue operation and as a 'wait' semaphore in another queue operation. For example, lets
             * say we have semaphore S and queue operations A and B that we want to execute in order. What we tell Vulkan
             * is that A will 'signal' semaphore S when it finishes executing, and B will 'wait' on semaphore S before
             * it begins executing. When A finishes, semaphore S will be signaled, while B wont start until semaphore S
             * is signaled. After B begins executing, semaphore S is automatically reset back to being unsignaled,
             * allowing it to be used again
             *
             * Note that, the waiting only happens on the GPU. The CPU continues running without blocking
            */
            void createSemaphore (void) {
                VkSemaphoreCreateInfo createInfo;
                createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                createInfo.pNext = nullptr;
                createInfo.flags = m_semaphoreInfo.meta.createFlags;

                auto result = vkCreateSemaphore (*m_semaphoreInfo.resource.logDeviceObj->getLogDevice(),
                                                  &createInfo,
                                                  nullptr,
                                                  &m_semaphoreInfo.resource.semaphore);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_semaphoreInfo.resource.logObj) << "[?] Semaphore"
                                                                << " "
                                                                << "[" << string_VkResult (result) << "]"
                                                                << std::endl;
                    throw std::runtime_error ("[?] Semaphore");
                }
                LOG_INFO (m_semaphoreInfo.resource.logObj)      << "[O] Semaphore"
                                                                << std::endl;
            }

            void destroySemaphore (void) {
                vkDestroySemaphore  (*m_semaphoreInfo.resource.logDeviceObj->getLogDevice(),
                                      m_semaphoreInfo.resource.semaphore,
                                      nullptr);
                LOG_INFO (m_semaphoreInfo.resource.logObj) << "[X] Semaphore"
                                                           << std::endl;
            }

            void onAttach (void) override {
                createSemaphore();
            }

            void onDetach (void) override {
                destroySemaphore();
            }

            void onUpdate (const float frameDelta) override {
                static_cast <void> (frameDelta);
                /* Do nothing */
            }

            ~VKSemaphore (void) {
                if (m_semaphoreInfo.state.logObjCreated)
                    delete m_semaphoreInfo.resource.logObj;
            }
    };
}   // namespace Renderer