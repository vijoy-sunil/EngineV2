#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <limits>
#include <algorithm>
#include <vulkan/vk_enum_string_helper.h>
#include "../Collection/CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"
#include "VKWindow.h"
#include "VKSurface.h"
#include "VKPhyDevice.h"
#include "VKLogDevice.h"
#include "VKHelper.h"

namespace Renderer {
    class VKSwapChain: public Collection::CNTypeInstanceBase {
        private:
            struct SwapChainInfo {
                struct Meta {
                    uint32_t imagesCount;
                    uint32_t layersCount;
                    VkExtent2D extent;
                    VkImageUsageFlags usages;
                    VkSurfaceFormatKHR surfaceFormat;
                    VkPresentModeKHR presentMode;
                    std::vector <uint32_t> queueFamilyIndices;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKWindow*    windowObj;
                    VKSurface*   surfaceObj;
                    VKPhyDevice* phyDeviceObj;
                    VKLogDevice* logDeviceObj;
                    VkSwapchainKHR swapChain;
                } resource;
            } m_swapChainInfo;

            VkSurfaceCapabilitiesKHR getSurfaceCapabilities (void) {
                VkSurfaceCapabilitiesKHR capabilities;
                vkGetPhysicalDeviceSurfaceCapabilitiesKHR (*m_swapChainInfo.resource.phyDeviceObj->getPhyDevice(),
                                                           *m_swapChainInfo.resource.surfaceObj->getSurface(),
                                                           &capabilities);
                return capabilities;
            }

            uint32_t getSwapChainMinImagesCount (void) {
                auto capabilities = getSurfaceCapabilities();
                /* The implementation specifies the minimum number of images that the swap chain requires to function. But
                 * simply sticking to this minimum means that the application may sometimes have to wait on the driver to
                 * complete internal operations before we can acquire another image to render to. Therefore it is
                 * recommended to request at least one more image than the minimum
                 *
                 * Note that, we only specified the minimum number of images in the swap chain, so the implementation is
                 * allowed to create a swap chain with more
                */
                uint32_t minImagesCount = capabilities.minImageCount + 1;
                if (capabilities.maxImageCount > 0 && minImagesCount > capabilities.maxImageCount)
                    minImagesCount = capabilities.maxImageCount;

                return minImagesCount;
            }

            /* The extent is the resolution of the swap chain images and it's almost always exactly equal to the
             * resolution of the window that we're drawing to in pixels. Vulkan tells us to match the resolution of the
             * window by setting the width and height in the currentExtent member
             *
             * However, some window managers do allow us to differ here and this is indicated by setting the width and
             * height in currentExtent to a special value: the maximum value of uint32_t. In that case we'll pick the
             * resolution that best matches the window within the minImageExtent and maxImageExtent bounds
            */
            VkExtent2D getSwapChainExtentEXT (void) {
                auto capabilities = getSurfaceCapabilities();
                if (capabilities.currentExtent.width != std::numeric_limits <uint32_t> ::max())
                    return capabilities.currentExtent;
                else {
                    int width, height;
                    /* Note that, the resolution that we specified earlier when creating the window is measured in
                     * screen coordinates, but, the swap chain extent must be specified in pixels. Unfortunately, if
                     * you are using a high DPI display (like a retina display), screen coordinates don't correspond to
                     * pixels. Instead, due to the higher pixel density, the resolution of the window in pixel will be
                     * larger than the resolution in screen coordinates
                     *
                     * So if vulkan doesn't fix the swap extent for us, we can't just use the window resolution we
                     * specified before. Instead, we must use glfwGetFramebufferSize to query the resolution of the
                     * window in pixel before matching it against the minimum and maximum image extent
                    */
                    glfwGetFramebufferSize (m_swapChainInfo.resource.windowObj->getWindow(), &width, &height);

                    auto actualExtent = VkExtent2D {
                        static_cast <uint32_t> (width),
                        static_cast <uint32_t> (height)
                    };

                    actualExtent.width  = std::clamp (actualExtent.width,
                                                      capabilities.minImageExtent.width,
                                                      capabilities.maxImageExtent.width);
                    actualExtent.height = std::clamp (actualExtent.height,
                                                      capabilities.minImageExtent.height,
                                                      capabilities.maxImageExtent.height);
                    return actualExtent;
                }
            }

            bool isSurfaceFormatSupported (const VkSurfaceFormatKHR surfaceFormat) {
                std::vector <VkSurfaceFormatKHR> availableSurfaceFormats;
                uint32_t surfaceFormatsCount;
                vkGetPhysicalDeviceSurfaceFormatsKHR     (*m_swapChainInfo.resource.phyDeviceObj->getPhyDevice(),
                                                          *m_swapChainInfo.resource.surfaceObj->getSurface(),
                                                          &surfaceFormatsCount,
                                                          nullptr);
                if (surfaceFormatsCount != 0) {
                    availableSurfaceFormats.resize (surfaceFormatsCount);
                    vkGetPhysicalDeviceSurfaceFormatsKHR (*m_swapChainInfo.resource.phyDeviceObj->getPhyDevice(),
                                                          *m_swapChainInfo.resource.surfaceObj->getSurface(),
                                                          &surfaceFormatsCount,
                                                          availableSurfaceFormats.data());
                }

                LOG_INFO (m_swapChainInfo.resource.logObj)     << "Available surface formats"
                                                               << std::endl;
                for (auto const& availableSurfaceFormat: availableSurfaceFormats)
                    LOG_INFO (m_swapChainInfo.resource.logObj) << "[" << string_VkFormat
                                                                         (availableSurfaceFormat.format)
                                                               << "]"
                                                               << " "
                                                               << "[" << string_VkColorSpaceKHR
                                                                         (availableSurfaceFormat.colorSpace)
                                                               << "]"
                                                               << std::endl;
                LOG_INFO (m_swapChainInfo.resource.logObj)     << LINE_BREAK
                                                               << std::endl;

                LOG_INFO (m_swapChainInfo.resource.logObj)     << "Required surface format"
                                                               << std::endl;
                LOG_INFO (m_swapChainInfo.resource.logObj)     << "[" << string_VkFormat
                                                                         (surfaceFormat.format)
                                                               << "]"
                                                               << " "
                                                               << "[" << string_VkColorSpaceKHR
                                                                         (surfaceFormat.colorSpace)
                                                               << "]"
                                                               << std::endl;
                LOG_INFO (m_swapChainInfo.resource.logObj)     << LINE_BREAK
                                                               << std::endl;

                for (auto const& availableSurfaceFormat: availableSurfaceFormats) {
                    if (availableSurfaceFormat.format     == surfaceFormat.format &&
                        availableSurfaceFormat.colorSpace == surfaceFormat.colorSpace) {
                        return true;
                    }
                }
                return false;
            }

            bool isPresentModeSupported (const VkPresentModeKHR presentMode) {
                std::vector <VkPresentModeKHR> availablePresentModes;
                uint32_t presentModesCount;
                vkGetPhysicalDeviceSurfacePresentModesKHR     (*m_swapChainInfo.resource.phyDeviceObj->getPhyDevice(),
                                                               *m_swapChainInfo.resource.surfaceObj->getSurface(),
                                                               &presentModesCount,
                                                               nullptr);
                if (presentModesCount != 0) {
                    availablePresentModes.resize (presentModesCount);
                    vkGetPhysicalDeviceSurfacePresentModesKHR (*m_swapChainInfo.resource.phyDeviceObj->getPhyDevice(),
                                                               *m_swapChainInfo.resource.surfaceObj->getSurface(),
                                                               &presentModesCount,
                                                               availablePresentModes.data());
                }

                LOG_INFO (m_swapChainInfo.resource.logObj)     << "Available present modes"
                                                               << std::endl;
                for (auto const& availablePresentMode: availablePresentModes)
                    LOG_INFO (m_swapChainInfo.resource.logObj) << "[" << string_VkPresentModeKHR
                                                                         (availablePresentMode)
                                                               << "]"
                                                               << std::endl;
                LOG_INFO (m_swapChainInfo.resource.logObj)     << LINE_BREAK
                                                               << std::endl;

                LOG_INFO (m_swapChainInfo.resource.logObj)     << "Required present mode"
                                                               << std::endl;
                LOG_INFO (m_swapChainInfo.resource.logObj)     << "[" << string_VkPresentModeKHR
                                                                         (presentMode)
                                                               << "]"
                                                               << std::endl;
                LOG_INFO (m_swapChainInfo.resource.logObj)     << LINE_BREAK
                                                               << std::endl;

                for (auto const& availablePresentMode: availablePresentModes) {
                    if (availablePresentMode == presentMode)
                        return true;
                }
                return false;
            }

            void createSwapChain (void) {
                VkSwapchainCreateInfoKHR createInfo;
                createInfo.sType                     = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
                createInfo.pNext                     = nullptr;
                createInfo.flags                     = 0;
                createInfo.minImageCount             = getSwapChainMinImagesCount();
                createInfo.imageArrayLayers          = m_swapChainInfo.meta.layersCount;
                createInfo.imageExtent               = m_swapChainInfo.meta.extent;
                createInfo.imageUsage                = m_swapChainInfo.meta.usages;
                createInfo.imageFormat               = m_swapChainInfo.meta.surfaceFormat.format;
                createInfo.imageColorSpace           = m_swapChainInfo.meta.surfaceFormat.colorSpace;
                createInfo.presentMode               = m_swapChainInfo.meta.presentMode;

                auto& queueFamilyIndices             = m_swapChainInfo.meta.queueFamilyIndices;
                if (isIndicesUnique (queueFamilyIndices)) {
                    createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
                    createInfo.queueFamilyIndexCount = static_cast <uint32_t> (queueFamilyIndices.size());
                    createInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
                }
                else {
                    createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
                    createInfo.queueFamilyIndexCount = 0;
                    createInfo.pQueueFamilyIndices   = nullptr;
                }

                createInfo.surface                   = *m_swapChainInfo.resource.surfaceObj->getSurface();
                createInfo.preTransform              = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
                createInfo.compositeAlpha            = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
                createInfo.clipped                   = VK_TRUE;
                createInfo.oldSwapchain              = nullptr;

                auto result = vkCreateSwapchainKHR (*m_swapChainInfo.resource.logDeviceObj->getLogDevice(),
                                                     &createInfo,
                                                     nullptr,
                                                     &m_swapChainInfo.resource.swapChain);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_swapChainInfo.resource.logObj) << "[?] Swap chain"
                                                                << " "
                                                                << "[" << string_VkResult (result) << "]"
                                                                << std::endl;
                    throw std::runtime_error ("[?] Swap chain");
                }
                LOG_INFO (m_swapChainInfo.resource.logObj)      << "[O] Swap chain"
                                                                << std::endl;

                /* Save swap chain size */
                vkGetSwapchainImagesKHR (*m_swapChainInfo.resource.logDeviceObj->getLogDevice(),
                                          m_swapChainInfo.resource.swapChain,
                                          &m_swapChainInfo.meta.imagesCount,
                                          nullptr);
            }

            void destroySwapChain (void) {
                vkDestroySwapchainKHR (*m_swapChainInfo.resource.logDeviceObj->getLogDevice(),
                                        m_swapChainInfo.resource.swapChain,
                                        nullptr);
                LOG_INFO (m_swapChainInfo.resource.logObj) << "[X] Swap chain"
                                                           << std::endl;
            }

        public:
            VKSwapChain (Log::LGImpl* logObj,
                         VKWindow*    windowObj,
                         VKSurface*   surfaceObj,
                         VKPhyDevice* phyDeviceObj,
                         VKLogDevice* logDeviceObj) {

                m_swapChainInfo = {};

                if (logObj == nullptr) {
                    m_swapChainInfo.resource.logObj     = new Log::LGImpl();
                    m_swapChainInfo.state.logObjCreated = true;

                    m_swapChainInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
                    LOG_WARNING (m_swapChainInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                  << std::endl;
                }
                else {
                    m_swapChainInfo.resource.logObj     = logObj;
                    m_swapChainInfo.state.logObjCreated = false;
                }

                if (windowObj    == nullptr || surfaceObj   == nullptr ||
                    phyDeviceObj == nullptr || logDeviceObj == nullptr) {

                    LOG_ERROR (m_swapChainInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_swapChainInfo.resource.windowObj      = windowObj;
                m_swapChainInfo.resource.surfaceObj     = surfaceObj;
                m_swapChainInfo.resource.phyDeviceObj   = phyDeviceObj;
                m_swapChainInfo.resource.logDeviceObj   = logDeviceObj;
            }

            void initSwapChainInfo (const VkImageUsageFlags imageUsages,
                                    const VkSurfaceFormatKHR surfaceFormat,
                                    const VkPresentModeKHR presentMode,
                                    const std::vector <uint32_t> queueFamilyIndices) {

                m_swapChainInfo.meta.imagesCount                  = 0;
                m_swapChainInfo.meta.layersCount                  = 1;
                m_swapChainInfo.meta.extent                       = getSwapChainExtentEXT();
                m_swapChainInfo.meta.usages                       = imageUsages;

                if (!isSurfaceFormatSupported (surfaceFormat)) {
                    LOG_WARNING (m_swapChainInfo.resource.logObj) << "Surface format not supported, setting to default"
                                                                  << std::endl;
                    /* We will settle with the first surface format in the list */
                    m_swapChainInfo.meta.surfaceFormat.format     = VK_FORMAT_B8G8R8A8_UNORM;
                    m_swapChainInfo.meta.surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                }
                else
                    m_swapChainInfo.meta.surfaceFormat            = surfaceFormat;

                if (!isPresentModeSupported (presentMode)) {
                    LOG_WARNING (m_swapChainInfo.resource.logObj) << "Present mode not supported, setting to default"
                                                                  << std::endl;
                    /* VK_PRESENT_MODE_FIFO_KHR is guaranteed to be available */
                    m_swapChainInfo.meta.presentMode              = VK_PRESENT_MODE_FIFO_KHR;
                }
                else
                    m_swapChainInfo.meta.presentMode              = presentMode;

                m_swapChainInfo.meta.queueFamilyIndices           = queueFamilyIndices;
                m_swapChainInfo.resource.swapChain                = nullptr;
            }

            uint32_t getSwapChainImagesCount (void) {
                return m_swapChainInfo.meta.imagesCount;
            }

            VkExtent2D* getSwapChainExtent (void) {
                return &m_swapChainInfo.meta.extent;
            }

            VkFormat getSwapChainFormat (void) {
                return m_swapChainInfo.meta.surfaceFormat.format;
            }

            VkSwapchainKHR* getSwapChain (void) {
                return &m_swapChainInfo.resource.swapChain;
            }

            std::vector <VkImage> getSwapChainImages (void) {
                std::vector <VkImage> images (m_swapChainInfo.meta.imagesCount);
                vkGetSwapchainImagesKHR (*m_swapChainInfo.resource.logDeviceObj->getLogDevice(),
                                          m_swapChainInfo.resource.swapChain,
                                          &m_swapChainInfo.meta.imagesCount,
                                          images.data());
                return images;
            }

            void onAttach (void) override {
                createSwapChain();
            }

            void onDetach (void) override {
                destroySwapChain();
            }

            void onUpdate (const float frameDelta) override {
                static_cast <void> (frameDelta);
                /* Do nothing */
            }

            ~VKSwapChain (void) {
                if (m_swapChainInfo.state.logObjCreated)
                    delete m_swapChainInfo.resource.logObj;
            }
    };
}   // namespace Renderer