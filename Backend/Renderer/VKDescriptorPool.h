#pragma once
#include "../Common.h"
#include "../Collection/CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"
#include "VKLogDevice.h"

namespace Renderer {
    class VKDescriptorPool: public Collection::CNTypeInstanceBase {
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

            void createDescriptorPool (void) {
                auto& meta               = m_descriptorPoolInfo.meta;
                auto& resource           = m_descriptorPoolInfo.resource;

                VkDescriptorPoolCreateInfo createInfo;
                createInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
                createInfo.pNext         = nullptr;
                createInfo.flags         = meta.createFlags;
                createInfo.poolSizeCount = static_cast <uint32_t> (meta.poolSizes.size());
                createInfo.pPoolSizes    = meta.poolSizes.data();
                createInfo.maxSets       = meta.maxDescriptorSets;

                auto result = vkCreateDescriptorPool (*resource.logDeviceObj->getLogDevice(),
                                                       &createInfo,
                                                       nullptr,
                                                       &resource.pool);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (resource.logObj) << "[?] Descriptor pool"
                                                << " "
                                                << "[" << string_VkResult (result) << "]"
                                                << std::endl;
                    throw std::runtime_error ("[?] Descriptor pool");
                }
                LOG_INFO (resource.logObj)      << "[O] Descriptor pool"
                                                << std::endl;
            }

            void destroyDescriptorPool (void) {
                auto& resource = m_descriptorPoolInfo.resource;
                vkDestroyDescriptorPool (*resource.logDeviceObj->getLogDevice(),
                                          resource.pool,
                                          nullptr);
                LOG_INFO (resource.logObj) << "[X] Descriptor pool"
                                           << std::endl;
            }

        public:
            VKDescriptorPool (Log::LGImpl* logObj,
                              VKLogDevice* logDeviceObj) {

                m_descriptorPoolInfo = {};

                if (logObj == nullptr) {
                    m_descriptorPoolInfo.resource.logObj     = new Log::LGImpl();
                    m_descriptorPoolInfo.state.logObjCreated = true;

                    m_descriptorPoolInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
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

                auto& meta                         = m_descriptorPoolInfo.meta;
                meta.createFlags                   = createFlags;
                meta.poolSizes                     = {};
                meta.maxDescriptorSets             = maxDescriptorSets;
                m_descriptorPoolInfo.resource.pool = nullptr;
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

            void onAttach (void) override {
                createDescriptorPool();
            }

            void onDetach (void) override {
                destroyDescriptorPool();
            }

            void onUpdate (void) override {
                /* Do nothing */
            }

            ~VKDescriptorPool (void) {
                if (m_descriptorPoolInfo.state.logObjCreated)
                    delete m_descriptorPoolInfo.resource.logObj;
            }
    };
}   // namespace Renderer