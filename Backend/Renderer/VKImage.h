#pragma once
#include "../Common.h"
#include "../Collection/CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"
#include "VKPhyDevice.h"
#include "VKLogDevice.h"
#include "VKHelper.h"

namespace Renderer {
    class VKImage: public Collection::CNTypeInstanceBase {
        private:
            struct ImageInfo {
                struct Meta {
                    uint32_t width;
                    uint32_t height;
                    uint32_t mipLevels;
                    uint32_t baseArrayLayer;
                    uint32_t layersCount;
                    VkImageCreateFlags createFlags;
                    VkImageLayout initialLayout;
                    VkFormat format;
                    VkImageUsageFlags usages;
                    VkSampleCountFlagBits samplesCount;
                    VkImageTiling tiling;
                    VkMemoryPropertyFlags memoryProperties;
                    std::vector <uint32_t> queueFamilyIndices;
                    VkImageAspectFlags aspectFlags;
                    VkImageViewType viewType;
                } meta;

                struct State {
                    bool onlyViewCreated;
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

            void createImage (void) {
                auto& meta                           = m_imageInfo.meta;
                auto& resource                       = m_imageInfo.resource;

                VkImageCreateInfo createInfo;
                createInfo.sType                     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                createInfo.pNext                     = nullptr;
                createInfo.flags                     = meta.createFlags;
                createInfo.imageType                 = VK_IMAGE_TYPE_2D;
                createInfo.extent.width              = meta.width;
                createInfo.extent.height             = meta.height;
                createInfo.extent.depth              = 1;
                createInfo.mipLevels                 = meta.mipLevels;
                createInfo.arrayLayers               = meta.layersCount;
                createInfo.initialLayout             = meta.initialLayout;
                createInfo.format                    = meta.format;
                createInfo.usage                     = meta.usages;
                createInfo.samples                   = meta.samplesCount;
                createInfo.tiling                    = meta.tiling;

                if (isIndicesUnique (meta.queueFamilyIndices)) {
                    createInfo.sharingMode           = VK_SHARING_MODE_CONCURRENT;
                    createInfo.queueFamilyIndexCount = static_cast <uint32_t> (meta.queueFamilyIndices.size());
                    createInfo.pQueueFamilyIndices   = meta.queueFamilyIndices.data();
                }
                else {
                    createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
                    createInfo.queueFamilyIndexCount = 0;
                    createInfo.pQueueFamilyIndices   = nullptr;
                }

                auto result = vkCreateImage (*resource.logDeviceObj->getLogDevice(),
                                              &createInfo,
                                              nullptr,
                                              &resource.image);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (resource.logObj) << "[?] Image"
                                                << " "
                                                << "[" << string_VkResult (result) << "]"
                                                << std::endl;
                    throw std::runtime_error ("[?] Image");
                }
                LOG_INFO (resource.logObj)      << "[O] Image"
                                                << std::endl;

                VkMemoryRequirements memoryRequirements;
                vkGetImageMemoryRequirements (*resource.logDeviceObj->getLogDevice(),
                                               resource.image,
                                               &memoryRequirements);

                VkMemoryAllocateInfo allocInfo;
                allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.pNext           = nullptr;
                allocInfo.allocationSize  = memoryRequirements.size;
                allocInfo.memoryTypeIndex = getMemoryTypeIdx (*resource.phyDeviceObj->getPhyDevice(),
                                                               memoryRequirements.memoryTypeBits,
                                                               meta.memoryProperties);

                result = vkAllocateMemory (*resource.logDeviceObj->getLogDevice(),
                                            &allocInfo,
                                            nullptr,
                                            &resource.memory);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (resource.logObj) << "[?] Image memory"
                                                << " "
                                                << "[" << string_VkResult (result) << "]"
                                                << std::endl;
                    throw std::runtime_error ("[?] Image memory");
                }
                LOG_INFO (resource.logObj)      << "[O] Image memory"
                                                << std::endl;

                vkBindImageMemory (*resource.logDeviceObj->getLogDevice(),
                                    resource.image,
                                    resource.memory,
                                    0);
            }

            void createImageView (void) {
                auto& meta                                 = m_imageInfo.meta;
                auto& resource                             = m_imageInfo.resource;

                VkImageViewCreateInfo createInfo;
                createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                createInfo.pNext                           = nullptr;
                createInfo.flags                           = 0;

                createInfo.subresourceRange.baseMipLevel   = 0;
                createInfo.subresourceRange.levelCount     = meta.mipLevels;
                createInfo.subresourceRange.baseArrayLayer = meta.baseArrayLayer;
                createInfo.subresourceRange.layerCount     = meta.layersCount;
                createInfo.subresourceRange.aspectMask     = meta.aspectFlags;

                createInfo.format                          = meta.format;
                createInfo.viewType                        = meta.viewType;
                createInfo.image                           = resource.image;
                createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;

                auto result = vkCreateImageView (*resource.logDeviceObj->getLogDevice(),
                                                  &createInfo,
                                                  nullptr,
                                                  &resource.view);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (resource.logObj) << "[?] Image view"
                                                << " "
                                                << "[" << string_VkResult (result) << "]"
                                                << std::endl;
                    throw std::runtime_error ("[?] Image view");
                }
                LOG_INFO (resource.logObj)      << "[O] Image view"
                                                << std::endl;
            }

            void destroyImage (void) {
                auto& resource = m_imageInfo.resource;
                vkDestroyImage (*resource.logDeviceObj->getLogDevice(),
                                 resource.image,
                                 nullptr);
                LOG_INFO (resource.logObj) << "[X] Image"
                                           << std::endl;

                vkFreeMemory   (*resource.logDeviceObj->getLogDevice(),
                                 resource.memory,
                                 nullptr);
                LOG_INFO (resource.logObj) << "[X] Image memory"
                                           << std::endl;
            }

            void destroyImageView (void) {
                auto& resource = m_imageInfo.resource;
                vkDestroyImageView (*resource.logDeviceObj->getLogDevice(),
                                     resource.view,
                                     nullptr);
                LOG_INFO (resource.logObj) << "[X] Image view"
                                           << std::endl;
            }

        public:
            VKImage (Log::LGImpl* logObj,
                     VKPhyDevice* phyDeviceObj,
                     VKLogDevice* logDeviceObj) {

                m_imageInfo = {};

                if (logObj == nullptr) {
                    m_imageInfo.resource.logObj     = new Log::LGImpl();
                    m_imageInfo.state.logObjCreated = true;

                    m_imageInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
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
                                const uint32_t baseArrayLayer,
                                const uint32_t layersCount,
                                const VkImageCreateFlags createFlags,
                                const VkImageLayout initialImageLayout,
                                const VkFormat format,
                                const VkImageUsageFlags imageUsages,
                                const VkSampleCountFlagBits samplesCount,
                                const VkImageTiling tiling,
                                const VkMemoryPropertyFlags memoryProperties,
                                const std::vector <uint32_t> queueFamilyIndices,
                                const VkImageAspectFlags aspectFlags,
                                const VkImageViewType viewType) {

                auto& meta                        = m_imageInfo.meta;
                auto& resource                    = m_imageInfo.resource;

                meta.width                        = width;
                meta.height                       = height;
                meta.mipLevels                    = mipLevels;
                meta.baseArrayLayer               = baseArrayLayer;
                meta.layersCount                  = layersCount;
                meta.createFlags                  = createFlags;
                meta.initialLayout                = initialImageLayout;
                meta.format                       = format;
                meta.usages                       = imageUsages;
                meta.samplesCount                 = samplesCount;
                meta.tiling                       = tiling;
                meta.memoryProperties             = memoryProperties;
                meta.queueFamilyIndices           = queueFamilyIndices;
                meta.aspectFlags                  = aspectFlags;
                meta.viewType                     = viewType;
                m_imageInfo.state.onlyViewCreated = false;

                resource.image                    = nullptr;
                resource.memory                   = nullptr;
                resource.view                     = nullptr;
            }

            void initImageInfo (const uint32_t mipLevels,
                                const uint32_t baseArrayLayer,
                                const uint32_t layersCount,
                                const VkFormat format,
                                const VkImageAspectFlags aspectFlags,
                                const VkImageViewType viewType,
                                const VkImage image) {

                auto& meta                        = m_imageInfo.meta;
                auto& resource                    = m_imageInfo.resource;

                meta.mipLevels                    = mipLevels;
                meta.baseArrayLayer               = baseArrayLayer;
                meta.layersCount                  = layersCount;
                meta.format                       = format;
                meta.aspectFlags                  = aspectFlags;
                meta.viewType                     = viewType;
                m_imageInfo.state.onlyViewCreated = true;

                resource.image                    = image;
                resource.view                     = nullptr;
            }

            VkExtent2D getImageExtent (void) {
                auto& meta = m_imageInfo.meta;
                return {meta.width, meta.height};
            }

            uint32_t getImageMipLevels (void) {
                return m_imageInfo.meta.mipLevels;
            }

            uint32_t getImageLayersCount (void) {
                return m_imageInfo.meta.layersCount;
            }

            VkFormat getImageFormat (void) {
                return m_imageInfo.meta.format;
            }

            VkSampleCountFlagBits getImageSamplesCount (void) {
                return m_imageInfo.meta.samplesCount;
            }

            VkImageAspectFlags getImageAspectFlags (void) {
                return m_imageInfo.meta.aspectFlags;
            }

            VkImage* getImage (void) {
                return &m_imageInfo.resource.image;
            }

            VkImageView* getImageView (void) {
                return &m_imageInfo.resource.view;
            }

            void onAttach (void) override {
                if (!m_imageInfo.state.onlyViewCreated)
                    createImage();
                createImageView();
            }

            void onDetach (void) override {
                destroyImageView();
                if (!m_imageInfo.state.onlyViewCreated)
                    destroyImage();
            }

            void onUpdate (void) override {
                /* Do nothing */
            }

            ~VKImage (void) {
                if (m_imageInfo.state.logObjCreated)
                    delete m_imageInfo.resource.logObj;
            }
    };
}   // namespace Renderer