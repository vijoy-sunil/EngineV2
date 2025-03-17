#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <set>
#include <vector>
#include <optional>
#include "../Collection/CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"
#include "VKInstance.h"
#include "VKSurface.h"

namespace Renderer {
    class VKPhyDevice: public Collection::CNTypeInstanceBase {
        private:
            struct PhyDeviceInfo {
                struct Meta {
                    std::vector <const char*> extensions;
                    std::optional <uint32_t> graphicsQueueFamilyIdx;
                    std::optional <uint32_t>  presentQueueFamilyIdx;
                    std::optional <uint32_t> transferQueueFamilyIdx;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKInstance*  instanceObj;
                    VKSurface*   surfaceObj;
                    VkPhysicalDevice device;
                } resource;
            } m_phyDeviceInfo;

            bool isDeviceExtensionsSupported (const VkPhysicalDevice phyDevice) {
                uint32_t extensionsCount = 0;
                vkEnumerateDeviceExtensionProperties (phyDevice, nullptr, &extensionsCount, nullptr);
                std::vector <VkExtensionProperties> availableExtensions (extensionsCount);
                vkEnumerateDeviceExtensionProperties (phyDevice, nullptr, &extensionsCount, availableExtensions.data());

                LOG_INFO (m_phyDeviceInfo.resource.logObj)     << "Available device extensions"
                                                               << std::endl;
                for (auto const& extension: availableExtensions)
                    LOG_INFO (m_phyDeviceInfo.resource.logObj) << "[" << extension.extensionName << "]"
                                                               << " "
                                                               << "[" << extension.specVersion   << "]"
                                                               << std::endl;
                LOG_INFO (m_phyDeviceInfo.resource.logObj)     << LINE_BREAK
                                                               << std::endl;

                LOG_INFO (m_phyDeviceInfo.resource.logObj)     << "Required device extensions"
                                                               << std::endl;
                for (auto const& extension: m_phyDeviceInfo.meta.extensions)
                    LOG_INFO (m_phyDeviceInfo.resource.logObj) << "[" << extension << "]"
                                                               << std::endl;
                LOG_INFO (m_phyDeviceInfo.resource.logObj)     << LINE_BREAK
                                                               << std::endl;

                std::set <std::string> requiredExtensions (m_phyDeviceInfo.meta.extensions.begin(),
                                                           m_phyDeviceInfo.meta.extensions.end());
                for (auto const& extension: availableExtensions)
                    requiredExtensions.erase (extension.extensionName);
                return requiredExtensions.empty();
            }

            bool populateQueueFamilyIndices (const VkPhysicalDevice phyDevice) {
                uint32_t queueFamiliesCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties (phyDevice, &queueFamiliesCount, nullptr);
                std::vector <VkQueueFamilyProperties> availableQueueFamilies (queueFamiliesCount);
                vkGetPhysicalDeviceQueueFamilyProperties (phyDevice, &queueFamiliesCount, availableQueueFamilies.data());

                struct QueueFamilyInfo {
                    bool graphicsQueueExists;
                    bool presentQueueExists;
                    bool transferQueueExists;
                };
                std::unordered_map <uint32_t, QueueFamilyInfo> queueFamilyInfoPool;
                uint32_t queueFamilyIdx = 0;

                for (auto const& queueFamily: availableQueueFamilies) {
                    auto flags              = queueFamily.queueFlags;
                    VkBool32 presentSupport = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR (phyDevice,
                                                          queueFamilyIdx,
                                                          *m_phyDeviceInfo.resource.surfaceObj->getSurface(),
                                                          &presentSupport);

                    if (flags & VK_QUEUE_GRAPHICS_BIT)
                        queueFamilyInfoPool[queueFamilyIdx].graphicsQueueExists = true;
                    if (presentSupport)
                        queueFamilyInfoPool[queueFamilyIdx].presentQueueExists  = true;
                    if (flags & VK_QUEUE_TRANSFER_BIT)
                        queueFamilyInfoPool[queueFamilyIdx].transferQueueExists = true;

                    ++queueFamilyIdx;
                }
                LOG_INFO (m_phyDeviceInfo.resource.logObj)     << "Available queue families"
                                                               << std::endl;
                for (auto const& [idx, info]: queueFamilyInfoPool) {
                    std::string graphicsQueueExists = info.graphicsQueueExists ? "[O]|G": "[X]|G";
                    std::string presentQueueExists  = info.presentQueueExists  ? "[O]|P": "[X]|P";
                    std::string transferQueueExists = info.transferQueueExists ? "[O]|T": "[X]|T";
                    LOG_INFO (m_phyDeviceInfo.resource.logObj) << idx
                                                               << " "
                                                               << "[" << graphicsQueueExists << "]"
                                                               << " "
                                                               << "[" << presentQueueExists  << "]"
                                                               << " "
                                                               << "[" << transferQueueExists << "]"
                                                               << std::endl;
                }
                LOG_INFO (m_phyDeviceInfo.resource.logObj)     << LINE_BREAK
                                                               << std::endl;
                /* Pick queue family indices */
                for (auto const& [idx, info]: queueFamilyInfoPool) {
                    if (info.graphicsQueueExists && !m_phyDeviceInfo.meta.graphicsQueueFamilyIdx.has_value())
                        m_phyDeviceInfo.meta.graphicsQueueFamilyIdx = idx;

                    if (info.presentQueueExists  && !m_phyDeviceInfo.meta.presentQueueFamilyIdx.has_value())
                        m_phyDeviceInfo.meta.presentQueueFamilyIdx  = idx;

                    if (info.transferQueueExists && !m_phyDeviceInfo.meta.transferQueueFamilyIdx.has_value())
                        m_phyDeviceInfo.meta.transferQueueFamilyIdx = idx;
                }

                auto& meta                         = m_phyDeviceInfo.meta;
                std::string graphicsQueueFamilyIdx = meta.graphicsQueueFamilyIdx.has_value() ?
                                                     std::to_string (meta.graphicsQueueFamilyIdx.value()): "X";
                std::string presentQueueFamilyIdx  = meta.presentQueueFamilyIdx. has_value() ?
                                                     std::to_string (meta.presentQueueFamilyIdx. value()): "X";
                std::string transferQueueFamilyIdx = meta.transferQueueFamilyIdx.has_value() ?
                                                     std::to_string (meta.transferQueueFamilyIdx.value()): "X";

                LOG_INFO (m_phyDeviceInfo.resource.logObj)     << "Selected queue families"
                                                               << std::endl;
                LOG_INFO (m_phyDeviceInfo.resource.logObj)     << "G"
                                                               << " "
                                                               << "[" << graphicsQueueFamilyIdx << "]"
                                                               << std::endl;
                LOG_INFO (m_phyDeviceInfo.resource.logObj)     << "P"
                                                               << " "
                                                               << "[" << presentQueueFamilyIdx  << "]"
                                                               << std::endl;
                LOG_INFO (m_phyDeviceInfo.resource.logObj)     << "T"
                                                               << " "
                                                               << "[" << transferQueueFamilyIdx << "]"
                                                               << std::endl;
                LOG_INFO (m_phyDeviceInfo.resource.logObj)     << LINE_BREAK
                                                               << std::endl;

                /* Return true if all queue family indices are populated */
                return m_phyDeviceInfo.meta.graphicsQueueFamilyIdx.has_value() &&
                       m_phyDeviceInfo.meta.presentQueueFamilyIdx. has_value() &&
                       m_phyDeviceInfo.meta.transferQueueFamilyIdx.has_value();
            }

            void populatePhyDeviceExtensionFeatures (const VkPhysicalDevice phyDevice,
                                                     const VkPhysicalDeviceFeatures features,
                                                     void* pNext) {

                /* If the VkPhysicalDevice[ExtensionName]Features structure is included in the pNext chain of the
                 * VkPhysicalDeviceFeatures2 structure passed to vkGetPhysicalDeviceFeatures2, it is filled in to
                 * indicate whether each corresponding feature is supported
                */
                VkPhysicalDeviceFeatures2 features2;
                features2.sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                features2.pNext    = pNext;
                features2.features = features;

                vkGetPhysicalDeviceFeatures2 (phyDevice, &features2);
            }

            bool isRequiredFeaturesSupported (const VkPhysicalDevice phyDevice) {
                /* Check if required features are supported
                 * (1) samplerAnisotropy
                 * (2) sampleRateShading
                 * (3) fillModeNonSolid
                 * (4) runtimeDescriptorArray
                 * (5) descriptorBindingSampledImageUpdateAfterBind
                */
                VkPhysicalDeviceFeatures features;
                vkGetPhysicalDeviceFeatures (phyDevice, &features);

                VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures;
                descriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
                descriptorIndexingFeatures.pNext = nullptr;
                populatePhyDeviceExtensionFeatures (phyDevice, features, &descriptorIndexingFeatures);

                return features.samplerAnisotropy                        &&
                       features.sampleRateShading                        &&
                       features.fillModeNonSolid                         &&
                       descriptorIndexingFeatures.runtimeDescriptorArray &&
                       descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind;
            }

            void createPhyDevice (void) {
                uint32_t phyDevicesCount = 0;
                vkEnumeratePhysicalDevices (*m_phyDeviceInfo.resource.instanceObj->getInstance(),
                                             &phyDevicesCount,
                                             nullptr);
                if (phyDevicesCount == 0) {
                    LOG_ERROR (m_phyDeviceInfo.resource.logObj) << "No phy devices found"
                                                                << std::endl;
                    throw std::runtime_error ("No phy devices found");
                }
                std::vector <VkPhysicalDevice> availablePhyDevices (phyDevicesCount);
                vkEnumeratePhysicalDevices (*m_phyDeviceInfo.resource.instanceObj->getInstance(),
                                             &phyDevicesCount,
                                             availablePhyDevices.data());

                /* Pick a phy device */
                for (auto const& phyDevice: availablePhyDevices) {
                    bool extensionsSupported        = isDeviceExtensionsSupported (phyDevice);
                    bool queueFamilyIndicesComplete = populateQueueFamilyIndices  (phyDevice);
                    bool requiredFeaturesSupported  = isRequiredFeaturesSupported (phyDevice);
                    /* Note that we are not checking if swap chain extension is supported and or adequate (i.e if there
                     * is at least one supported image format and one supported presentation mode given the window
                     * surface). However, the availability of a presentation queue, implies that the extension must be
                     * supported, hence why we will skip this step
                    */
                    if (extensionsSupported && queueFamilyIndicesComplete && requiredFeaturesSupported) {
                        m_phyDeviceInfo.resource.device = phyDevice;
                        break;
                    }
                }

                if (m_phyDeviceInfo.resource.device == nullptr) {
                    LOG_ERROR (m_phyDeviceInfo.resource.logObj) << "[?] Phy device"
                                                                << std::endl;
                    throw std::runtime_error ("[?] Phy device");
                }
                LOG_INFO (m_phyDeviceInfo.resource.logObj)      << "[O] Phy device"
                                                                << std::endl;
            }

            void destroyPhyDevice (void) {
                /* The phy device object will be implicitly destroyed when the instance is destroyed, so we won't need
                 * to do anything in the destroy function
                */
                LOG_INFO (m_phyDeviceInfo.resource.logObj) << "[X] Phy device"
                                                           << std::endl;
            }

        public:
            VKPhyDevice (Log::LGImpl* logObj,
                         VKInstance*  instanceObj,
                         VKSurface*   surfaceObj) {

                m_phyDeviceInfo = {};

                if (logObj == nullptr) {
                    m_phyDeviceInfo.resource.logObj     = new Log::LGImpl();
                    m_phyDeviceInfo.state.logObjCreated = true;

                    m_phyDeviceInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
                    LOG_WARNING (m_phyDeviceInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                  << std::endl;
                }
                else {
                    m_phyDeviceInfo.resource.logObj     = logObj;
                    m_phyDeviceInfo.state.logObjCreated = false;
                }

                if (instanceObj == nullptr || surfaceObj == nullptr) {
                    LOG_ERROR (m_phyDeviceInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_phyDeviceInfo.resource.instanceObj    = instanceObj;
                m_phyDeviceInfo.resource.surfaceObj     = surfaceObj;
            }

            void initPhyDeviceInfo (const std::vector <const char*> deviceExtensions) {
                m_phyDeviceInfo.meta.extensions = deviceExtensions;
                m_phyDeviceInfo.resource.device = nullptr;
            }

            std::vector <const char*>& getDeviceExtensions (void) {
                return m_phyDeviceInfo.meta.extensions;
            }

            uint32_t getGraphicsQueueFamilyIdx (void) {
                return m_phyDeviceInfo.meta.graphicsQueueFamilyIdx.value();
            }

            uint32_t getPresentQueueFamilyIdx (void) {
                return m_phyDeviceInfo.meta.presentQueueFamilyIdx.value();
            }

            uint32_t getTransferQueueFamilyIdx (void) {
                return m_phyDeviceInfo.meta.transferQueueFamilyIdx.value();
            }

            VkPhysicalDevice* getPhyDevice (void) {
                return &m_phyDeviceInfo.resource.device;
            }

            void onAttach (void) override {
                createPhyDevice();
            }

            void onDetach (void) override {
                destroyPhyDevice();
            }

            void onUpdate (const float frameDelta) override {
                static_cast <void> (frameDelta);
                /* Do nothing */
            }

            ~VKPhyDevice (void) {
                if (m_phyDeviceInfo.state.logObjCreated)
                    delete m_phyDeviceInfo.resource.logObj;
            }
    };
}   // namespace Renderer
