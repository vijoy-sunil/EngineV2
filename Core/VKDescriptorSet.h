#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include "../Backend/Layer/LYInstanceBase.h"
#include "../Backend/Log/LGImpl.h"
#include "VKLogDevice.h"

namespace Core {
    class VKDescriptorSet: public Layer::LYInstanceBase {
        private:
            struct DescriptorSetInfo {
                struct Meta {
                    uint32_t setsCount;
                    VkDescriptorSetLayout layout;
                    VkDescriptorPool pool;
                    std::vector <VkWriteDescriptorSet> writeSets;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKLogDevice* logDeviceObj;
                    std::vector <VkDescriptorSet> sets;
                } resource;
            } m_descriptorSetInfo;

        public:
            VKDescriptorSet (Log::LGImpl* logObj,
                             VKLogDevice* logDeviceObj) {

                m_descriptorSetInfo = {};

                if (logObj == nullptr) {
                    m_descriptorSetInfo.resource.logObj     = new Log::LGImpl();
                    m_descriptorSetInfo.state.logObjCreated = true;

                    m_descriptorSetInfo.resource.logObj->initLogInfo ("Build/Log/Core", __FILE__);
                    LOG_WARNING (m_descriptorSetInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                      << std::endl;
                }
                else {
                    m_descriptorSetInfo.resource.logObj     = logObj;
                    m_descriptorSetInfo.state.logObjCreated = false;
                }

                if (logDeviceObj == nullptr) {
                    LOG_ERROR (m_descriptorSetInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                    << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_descriptorSetInfo.resource.logDeviceObj   = logDeviceObj;
            }

            void initDescriptorSetInfo (const uint32_t descriptorSetsCount,
                                        const VkDescriptorSetLayout descriptorSetLayout,
                                        const VkDescriptorPool descriptorPool) {

                m_descriptorSetInfo.meta.setsCount = descriptorSetsCount;
                m_descriptorSetInfo.meta.layout    = descriptorSetLayout;
                m_descriptorSetInfo.meta.pool      = descriptorPool;
                m_descriptorSetInfo.meta.writeSets = {};
                m_descriptorSetInfo.resource.sets  = {};
            }

            VkDescriptorBufferInfo createDescriptorBufferInfo (const VkBuffer buffer,
                                                               const VkDeviceSize offset,
                                                               const VkDeviceSize range) {

                VkDescriptorBufferInfo descriptorBufferInfo;
                descriptorBufferInfo.buffer = buffer;
                descriptorBufferInfo.offset = offset;
                descriptorBufferInfo.range  = range;
                return descriptorBufferInfo;
            }

            VkDescriptorImageInfo createDescriptorImageInfo (const VkSampler sampler,
                                                             const VkImageView imageView,
                                                             const VkImageLayout imageLayout) {

                VkDescriptorImageInfo descriptorImageInfo;
                descriptorImageInfo.sampler     = sampler;
                descriptorImageInfo.imageView   = imageView;
                descriptorImageInfo.imageLayout = imageLayout;
                return descriptorImageInfo;
            }

            void addWriteDescriptorSet (const uint32_t bindingNumber,
                                        const uint32_t descriptorsCount,
                                        const uint32_t arrayElement,
                                        const VkDescriptorType descriptorType,
                                        const VkDescriptorSet descriptorSet,
                                        const std::vector <VkDescriptorBufferInfo>& descriptorBufferInfos,
                                        const std::vector <VkDescriptorImageInfo>&  descriptorImageInfos) {

                VkWriteDescriptorSet writeDescriptorSet;
                writeDescriptorSet.sType            = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet.pNext            = nullptr;
                writeDescriptorSet.dstBinding       = bindingNumber;
                writeDescriptorSet.descriptorCount  = descriptorsCount;
                writeDescriptorSet.dstArrayElement  = arrayElement;
                writeDescriptorSet.descriptorType   = descriptorType;
                writeDescriptorSet.dstSet           = descriptorSet;
                writeDescriptorSet.pBufferInfo      = descriptorBufferInfos.data();
                writeDescriptorSet.pImageInfo       = descriptorImageInfos.data();
                writeDescriptorSet.pTexelBufferView = nullptr;

                m_descriptorSetInfo.meta.writeSets.push_back (writeDescriptorSet);
            }

            /* Note that vkUpdateDescriptorSets doesn't copy a buffer, for example, into the descriptor set, but rather
             * gives the descriptor set a pointer to the buffer described by VkDescriptorBufferInfo. So then this method
             * doesn't need to be called more than once for a descriptor set, since modifying the buffer that a
             * descriptor set points to will update what the descriptor set sees
            */
            void updateDescriptorSets (void) {
                vkUpdateDescriptorSets (*m_descriptorSetInfo.resource.logDeviceObj->getLogDevice(),
                                         static_cast <uint32_t> (m_descriptorSetInfo.meta.writeSets.size()),
                                         m_descriptorSetInfo.meta.writeSets.data(),
                                         0,
                                         nullptr);
            }

            std::vector <VkDescriptorSet>& getDescriptorSets (void) {
                return m_descriptorSetInfo.resource.sets;
            }

            void createDescriptorSets (void) {
                VkDescriptorSetAllocateInfo allocInfo;
                allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.pNext              = nullptr;
                allocInfo.descriptorSetCount = m_descriptorSetInfo.meta.setsCount;
                auto layouts                 = std::vector <VkDescriptorSetLayout> (m_descriptorSetInfo.meta.setsCount,
                                                                                    m_descriptorSetInfo.meta.layout);
                allocInfo.pSetLayouts        = layouts.data();
                allocInfo.descriptorPool     = m_descriptorSetInfo.meta.pool;

                m_descriptorSetInfo.resource.sets.resize (m_descriptorSetInfo.meta.setsCount);
                auto result = vkAllocateDescriptorSets   (*m_descriptorSetInfo.resource.logDeviceObj->getLogDevice(),
                                                           &allocInfo,
                                                           m_descriptorSetInfo.resource.sets.data());
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_descriptorSetInfo.resource.logObj) << "[?] Descriptor set(s)"
                                                                    << " "
                                                                    << "[" << string_VkResult (result) << "]"
                                                                    << std::endl;
                    throw std::runtime_error ("[?] Descriptor set(s)");
                }
                LOG_INFO (m_descriptorSetInfo.resource.logObj)      << "[O] Descriptor set(s)"
                                                                    << std::endl;
            }

            void destroyDescriptorSets (void) {
                /* We don't need to explicitly destroy descriptor sets, because they will be automatically freed when
                 * the descriptor pool is destroyed
                */
                LOG_INFO (m_descriptorSetInfo.resource.logObj) << "[X] Descriptor set(s)"
                                                               << std::endl;
            }

            ~VKDescriptorSet (void) {
                if (m_descriptorSetInfo.state.logObjCreated)
                    delete m_descriptorSetInfo.resource.logObj;
            }
    };
}   // namespace Core