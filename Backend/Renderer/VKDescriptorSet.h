#pragma once
#include "../Common.h"
#include "../Collection/CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"
#include "VKLogDevice.h"
#include "VKDescriptorPool.h"

namespace Renderer {
    class VKDescriptorSet: public Collection::CNTypeInstanceBase {
        private:
            struct DescriptorSetInfo {
                struct Meta {
                    uint32_t setsCount;
                    VkDescriptorSetLayout layout;
                    std::vector <VkWriteDescriptorSet> writeSets;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKLogDevice* logDeviceObj;
                    VKDescriptorPool* descPoolObj;
                    std::vector <VkDescriptorSet> sets;
                } resource;
            } m_descriptorSetInfo;

            void createDescriptorSets (void) {
                auto& meta                   = m_descriptorSetInfo.meta;
                auto& resource               = m_descriptorSetInfo.resource;

                VkDescriptorSetAllocateInfo allocInfo;
                allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.pNext              = nullptr;
                allocInfo.descriptorSetCount = meta.setsCount;

                auto layouts                 = std::vector <VkDescriptorSetLayout> (meta.setsCount, meta.layout);
                allocInfo.pSetLayouts        = layouts.data();
                allocInfo.descriptorPool     = *resource.descPoolObj->getDescriptorPool();

                resource.sets.resize (meta.setsCount);
                auto result = vkAllocateDescriptorSets (*resource.logDeviceObj->getLogDevice(),
                                                         &allocInfo,
                                                         resource.sets.data());
                if (result != VK_SUCCESS) {
                    LOG_ERROR (resource.logObj) << "[?] Descriptor set(s)"
                                                << " "
                                                << "[" << string_VkResult (result) << "]"
                                                << std::endl;
                    throw std::runtime_error ("[?] Descriptor set(s)");
                }
                LOG_INFO (resource.logObj)      << "[O] Descriptor set(s)"
                                                << std::endl;
            }

            void destroyDescriptorSets (void) {
                /* We don't need to explicitly destroy descriptor sets, because they will be automatically freed when
                 * the descriptor pool is destroyed
                */
                LOG_INFO (m_descriptorSetInfo.resource.logObj) << "[X] Descriptor set(s)"
                                                               << std::endl;
            }

        public:
            VKDescriptorSet (Log::LGImpl*      logObj,
                             VKLogDevice*      logDeviceObj,
                             VKDescriptorPool* descPoolObj) {

                m_descriptorSetInfo = {};

                if (logObj == nullptr) {
                    m_descriptorSetInfo.resource.logObj     = new Log::LGImpl();
                    m_descriptorSetInfo.state.logObjCreated = true;

                    m_descriptorSetInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
                    LOG_WARNING (m_descriptorSetInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                      << std::endl;
                }
                else {
                    m_descriptorSetInfo.resource.logObj     = logObj;
                    m_descriptorSetInfo.state.logObjCreated = false;
                }

                if (logDeviceObj == nullptr || descPoolObj == nullptr) {
                    LOG_ERROR (m_descriptorSetInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                    << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_descriptorSetInfo.resource.logDeviceObj   = logDeviceObj;
                m_descriptorSetInfo.resource.descPoolObj    = descPoolObj;
            }

            void initDescriptorSetInfo (const uint32_t descriptorSetsCount,
                                        const VkDescriptorSetLayout descriptorSetLayout) {

                auto& meta                        = m_descriptorSetInfo.meta;
                meta.setsCount                    = descriptorSetsCount;
                meta.layout                       = descriptorSetLayout;
                meta.writeSets                    = {};
                m_descriptorSetInfo.resource.sets = {};
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

            VkDescriptorImageInfo createDescriptorImageInfo (const VkImageView imageView,
                                                             const VkSampler sampler,
                                                             const VkImageLayout imageLayout) {

                VkDescriptorImageInfo descriptorImageInfo;
                descriptorImageInfo.imageView   = imageView;
                descriptorImageInfo.sampler     = sampler;
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
                auto& writeSets = m_descriptorSetInfo.meta.writeSets;
                vkUpdateDescriptorSets (*m_descriptorSetInfo.resource.logDeviceObj->getLogDevice(),
                                         static_cast <uint32_t> (writeSets.size()),
                                         writeSets.data(),
                                         0,
                                         nullptr);
            }

            std::vector <VkDescriptorSet>& getDescriptorSets (void) {
                return m_descriptorSetInfo.resource.sets;
            }

            void onAttach (void) override {
                createDescriptorSets();
            }

            void onDetach (void) override {
                destroyDescriptorSets();
            }

            void onUpdate (void) override {
                /* Do nothing */
            }

            ~VKDescriptorSet (void) {
                if (m_descriptorSetInfo.state.logObjCreated)
                    delete m_descriptorSetInfo.resource.logObj;
            }
    };
}   // namespace Renderer