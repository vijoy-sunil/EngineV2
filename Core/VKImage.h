#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include "../Backend/Layer/LYInstanceBase.h"
#include "../Backend/Log/LGImpl.h"
#include "VKPhyDevice.h"
#include "VKLogDevice.h"
#include "VKHelper.h"

namespace Core {
    class VKImage: public Layer::LYInstanceBase {
        private:
            struct ImageInfo {
                struct Meta {
                    uint32_t width;
                    uint32_t height;
                    uint32_t mipLevels;
                    uint32_t layersCount;
                    VkImageCreateFlags createFlags;
                    VkImageLayout initialLayout;
                    VkFormat format;
                    VkImageUsageFlags usage;
                    VkSampleCountFlagBits samplesCount;
                    VkImageTiling tiling;
                    VkMemoryPropertyFlags property;
                    std::vector <uint32_t> queueFamilyIndices;
                    VkImageAspectFlags aspectFlags;
                    VkImageViewType viewType;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKPhyDevice* phyDeviceObj;
                    VKLogDevice* logDeviceObj;
                    VkImage image;
                    VkDeviceMemory memory;
                    VkImageView view;
                } resource;
            } m_imageInfo;

        public:
            VKImage (Log::LGImpl* logObj,
                     VKPhyDevice* phyDeviceObj,
                     VKLogDevice* logDeviceObj) {

                m_imageInfo = {};

                if (logObj == nullptr) {
                    m_imageInfo.resource.logObj     = new Log::LGImpl();
                    m_imageInfo.state.logObjCreated = true;

                    m_imageInfo.resource.logObj->initLogInfo();
                    LOG_WARNING (m_imageInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                              << std::endl;
                }
                else {
                    m_imageInfo.resource.logObj     = logObj;
                    m_imageInfo.state.logObjCreated = false;
                }

                if (phyDeviceObj == nullptr || logDeviceObj == nullptr) {
                    LOG_ERROR (m_imageInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                            << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_imageInfo.resource.phyDeviceObj   = phyDeviceObj;
                m_imageInfo.resource.logDeviceObj   = logDeviceObj;
            }

            void initImageInfo (const uint32_t width,
                                const uint32_t height,
                                const uint32_t mipLevels,
                                const uint32_t layersCount,
                                const VkImageCreateFlags createFlags,
                                const VkImageLayout initialLayout,
                                const VkFormat format,
                                const VkImageUsageFlags imageUsage,
                                const VkSampleCountFlagBits samplesCount,
                                const VkImageTiling tiling,
                                const VkMemoryPropertyFlags property,
                                const std::vector <uint32_t> queueFamilyIndices,
                                const VkImageAspectFlags aspectFlags,
                                const VkImageViewType viewType) {

                m_imageInfo.meta.width              = width;
                m_imageInfo.meta.height             = height;
                m_imageInfo.meta.mipLevels          = mipLevels;
                m_imageInfo.meta.layersCount        = layersCount;
                m_imageInfo.meta.createFlags        = createFlags;
                m_imageInfo.meta.initialLayout      = initialLayout;
                m_imageInfo.meta.format             = format;
                m_imageInfo.meta.usage              = imageUsage;
                m_imageInfo.meta.samplesCount       = samplesCount;
                m_imageInfo.meta.tiling             = tiling;
                m_imageInfo.meta.property           = property;
                m_imageInfo.meta.queueFamilyIndices = queueFamilyIndices;
                m_imageInfo.meta.aspectFlags        = aspectFlags;
                m_imageInfo.meta.viewType           = viewType;
                m_imageInfo.resource.image          = nullptr;
                m_imageInfo.resource.memory         = nullptr;
                m_imageInfo.resource.view           = nullptr;
            }

            void initImageInfo (const uint32_t mipLevels,
                                const uint32_t layersCount,
                                const VkFormat format,
                                const VkImageAspectFlags aspectFlags,
                                const VkImageViewType viewType,
                                const VkImage image) {

                m_imageInfo.meta.mipLevels   = mipLevels;
                m_imageInfo.meta.layersCount = layersCount;
                m_imageInfo.meta.format      = format;
                m_imageInfo.meta.aspectFlags = aspectFlags;
                m_imageInfo.meta.viewType    = viewType;
                m_imageInfo.resource.image   = image;
                m_imageInfo.resource.view    = nullptr;
            }

            void createImage (void) {
                VkImageCreateInfo createInfo;
                createInfo.sType                     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                createInfo.pNext                     = nullptr;
                createInfo.flags                     = m_imageInfo.meta.createFlags;
                createInfo.imageType                 = VK_IMAGE_TYPE_2D;
                createInfo.extent.width              = m_imageInfo.meta.width;
                createInfo.extent.height             = m_imageInfo.meta.height;
                createInfo.extent.depth              = 1;
                createInfo.mipLevels                 = m_imageInfo.meta.mipLevels;
                createInfo.arrayLayers               = m_imageInfo.meta.layersCount;
                createInfo.initialLayout             = m_imageInfo.meta.initialLayout;
                createInfo.format                    = m_imageInfo.meta.format;
                createInfo.usage                     = m_imageInfo.meta.usage;
                createInfo.samples                   = m_imageInfo.meta.samplesCount;
                createInfo.tiling                    = m_imageInfo.meta.tiling;

                auto queueFamilyIndices              = m_imageInfo.meta.queueFamilyIndices;
                if (isIndicesUnique (queueFamilyIndices)) {
                    createInfo.sharingMode           = VK_SHARING_MODE_CONCURRENT;
                    createInfo.queueFamilyIndexCount = static_cast <uint32_t> (queueFamilyIndices.size());
                    createInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
                }
                else {
                    createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
                    createInfo.queueFamilyIndexCount = 0;
                    createInfo.pQueueFamilyIndices   = nullptr;
                }

                auto result = vkCreateImage (*m_imageInfo.resource.logDeviceObj->getLogDevice(),
                                              &createInfo,
                                              nullptr,
                                              &m_imageInfo.resource.image);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_imageInfo.resource.logObj) << "[?] Image"
                                                            << " "
                                                            << "[" << string_VkResult (result) << "]"
                                                            << std::endl;
                    throw std::runtime_error ("[?] Image");
                }
                LOG_INFO (m_imageInfo.resource.logObj)      << "[O] Image"
                                                            << std::endl;

                VkMemoryRequirements memoryRequirements;
                vkGetImageMemoryRequirements (*m_imageInfo.resource.logDeviceObj->getLogDevice(),
                                               m_imageInfo.resource.image,
                                               &memoryRequirements);

                VkMemoryAllocateInfo allocInfo;
                allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.pNext           = nullptr;
                allocInfo.allocationSize  = memoryRequirements.size;
                allocInfo.memoryTypeIndex = getMemoryTypeIdx (*m_imageInfo.resource.phyDeviceObj->getPhyDevice(),
                                                               memoryRequirements.memoryTypeBits,
                                                               m_imageInfo.meta.property);

                result = vkAllocateMemory (*m_imageInfo.resource.logDeviceObj->getLogDevice(),
                                            &allocInfo,
                                            nullptr,
                                            &m_imageInfo.resource.memory);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_imageInfo.resource.logObj) << "[?] Image memory"
                                                            << " "
                                                            << "[" << string_VkResult (result) << "]"
                                                            << std::endl;
                    throw std::runtime_error ("[?] Image memory");
                }
                LOG_INFO (m_imageInfo.resource.logObj)      << "[O] Image memory"
                                                            << std::endl;

                vkBindImageMemory (*m_imageInfo.resource.logDeviceObj->getLogDevice(),
                                    m_imageInfo.resource.image,
                                    m_imageInfo.resource.memory,
                                    0);
                createImageView();
            }

            void createImageView (void) {
                VkImageViewCreateInfo createInfo;
                createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                createInfo.pNext                           = nullptr;
                createInfo.flags                           = 0;

                createInfo.subresourceRange.baseMipLevel   = 0;
                createInfo.subresourceRange.levelCount     = m_imageInfo.meta.mipLevels;
                createInfo.subresourceRange.baseArrayLayer = 0;
                createInfo.subresourceRange.layerCount     = m_imageInfo.meta.layersCount;
                createInfo.subresourceRange.aspectMask     = m_imageInfo.meta.aspectFlags;

                createInfo.format                          = m_imageInfo.meta.format;
                createInfo.viewType                        = m_imageInfo.meta.viewType;
                createInfo.image                           = m_imageInfo.resource.image;
                createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;

                auto result = vkCreateImageView (*m_imageInfo.resource.logDeviceObj->getLogDevice(),
                                                  &createInfo,
                                                  nullptr,
                                                  &m_imageInfo.resource.view);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_imageInfo.resource.logObj) << "[?] Image view"
                                                            << " "
                                                            << "[" << string_VkResult (result) << "]"
                                                            << std::endl;
                    throw std::runtime_error ("[?] Image view");
                }
                LOG_INFO (m_imageInfo.resource.logObj)      << "[O] Image view"
                                                            << std::endl;
            }

            void destroyImage (void) {
                destroyImageView();
                vkDestroyImage (*m_imageInfo.resource.logDeviceObj->getLogDevice(),
                                 m_imageInfo.resource.image,
                                 nullptr);
                LOG_INFO (m_imageInfo.resource.logObj) << "[X] Image"
                                                       << std::endl;

                vkFreeMemory   (*m_imageInfo.resource.logDeviceObj->getLogDevice(),
                                m_imageInfo.resource.memory,
                                nullptr);
                LOG_INFO (m_imageInfo.resource.logObj) << "[X] Image memory"
                                                       << std::endl;
            }

            void destroyImageView (void) {
                vkDestroyImageView (*m_imageInfo.resource.logDeviceObj->getLogDevice(),
                                     m_imageInfo.resource.view,
                                     nullptr);
                LOG_INFO (m_imageInfo.resource.logObj) << "[X] Image view"
                                                       << std::endl;
            }

            ~VKImage (void) {
                if (m_imageInfo.state.logObjCreated)
                    delete m_imageInfo.resource.logObj;
            }
    };
}   // namespace Core