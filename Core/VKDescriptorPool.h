#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include "../Backend/Layer/LYInstanceBase.h"
#include "../Backend/Log/LGImpl.h"
#include "VKLogDevice.h"

namespace Core {
    class VKDescriptorPool: public Layer::LYInstanceBase {
        private:
            struct DescriptorPoolInfo {
                struct Meta {
                    VkDescriptorPoolCreateFlags createFlags;
                    std::vector <VkDescriptorPoolSize> poolSizes;
                    uint32_t maxDescriptorSets;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKLogDevice* logDeviceObj;
                    VkDescriptorPool pool;
                } resource;
            } m_descriptorPoolInfo;

        public:
            VKDescriptorPool (Log::LGImpl* logObj,
                              VKLogDevice* logDeviceObj) {

                m_descriptorPoolInfo = {};

                if (logObj == nullptr) {
                    m_descriptorPoolInfo.resource.logObj     = new Log::LGImpl();
                    m_descriptorPoolInfo.state.logObjCreated = true;

                    m_descriptorPoolInfo.resource.logObj->initLogInfo ("Build/Log/Core", __FILE__);
                    LOG_WARNING (m_descriptorPoolInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                       << std::endl;
                }
                else {
                    m_descriptorPoolInfo.resource.logObj     = logObj;
                    m_descriptorPoolInfo.state.logObjCreated = false;
                }

                if (logDeviceObj == nullptr) {
                    LOG_ERROR (m_descriptorPoolInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                     << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_descriptorPoolInfo.resource.logDeviceObj   = logDeviceObj;
            }

            void initDescriptorPoolInfo (const VkDescriptorPoolCreateFlags createFlags,
                                         const uint32_t maxDescriptorSets) {

                m_descriptorPoolInfo.meta.createFlags       = createFlags;
                m_descriptorPoolInfo.meta.poolSizes         = {};
                m_descriptorPoolInfo.meta.maxDescriptorSets = maxDescriptorSets;
                m_descriptorPoolInfo.resource.pool          = nullptr;
            }

            void addDescriptorPoolSize (const uint32_t descriptorsCount,
                                        const VkDescriptorType descriptorType) {

                VkDescriptorPoolSize poolSize;
                poolSize.descriptorCount = descriptorsCount;
                poolSize.type            = descriptorType;

                m_descriptorPoolInfo.meta.poolSizes.push_back (poolSize);
            }

            VkDescriptorPool* getDescriptorPool (void) {
                return &m_descriptorPoolInfo.resource.pool;
            }

            void createDescriptorPool (void) {
                VkDescriptorPoolCreateInfo createInfo;
                createInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                createInfo.pNext         = nullptr;
                createInfo.flags         = m_descriptorPoolInfo.meta.createFlags;
                createInfo.poolSizeCount = static_cast <uint32_t> (m_descriptorPoolInfo.meta.poolSizes.size());
                createInfo.pPoolSizes    = m_descriptorPoolInfo.meta.poolSizes.data();
                createInfo.maxSets       = m_descriptorPoolInfo.meta.maxDescriptorSets;

                auto result = vkCreateDescriptorPool (*m_descriptorPoolInfo.resource.logDeviceObj->getLogDevice(),
                                                       &createInfo,
                                                       nullptr,
                                                       &m_descriptorPoolInfo.resource.pool);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_descriptorPoolInfo.resource.logObj) << "[?] Descriptor pool"
                                                                     << " "
                                                                     << "[" << string_VkResult (result) << "]"
                                                                     << std::endl;
                    throw std::runtime_error ("[?] Descriptor pool");
                }
                LOG_INFO (m_descriptorPoolInfo.resource.logObj)      << "[O] Descriptor pool"
                                                                     << std::endl;
            }

            void destroyDescriptorPool (void) {
                vkDestroyDescriptorPool (*m_descriptorPoolInfo.resource.logDeviceObj->getLogDevice(),
                                          m_descriptorPoolInfo.resource.pool,
                                          nullptr);
                LOG_INFO (m_descriptorPoolInfo.resource.logObj) << "[X] Descriptor pool"
                                                                << std::endl;
            }

            ~VKDescriptorPool (void) {
                if (m_descriptorPoolInfo.state.logObjCreated)
                    delete m_descriptorPoolInfo.resource.logObj;
            }
    };
}   // namespace Core