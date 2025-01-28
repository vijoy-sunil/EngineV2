#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <set>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include "../Backend/Layer/LYInstanceBase.h"
#include "../Backend/Log/LGImpl.h"
#include "VKInstance.h"
#include "VKPhyDevice.h"

namespace Core {
    class VKLogDevice: public Layer::LYInstanceBase {
        private:
            struct LogDeviceInfo {
                struct Meta {
                    VkQueue graphicsQueue;
                    VkQueue  presentQueue;
                    VkQueue transferQueue;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKInstance*  instanceObj;
                    VKPhyDevice* phyDeviceObj;
                    VkDevice device;
                } resource;
            } m_logDeviceInfo;

        public:
            VKLogDevice (Log::LGImpl* logObj,
                         VKInstance*  instanceObj,
                         VKPhyDevice* phyDeviceObj) {

                m_logDeviceInfo = {};

                if (logObj == nullptr) {
                    m_logDeviceInfo.resource.logObj     = new Log::LGImpl();
                    m_logDeviceInfo.state.logObjCreated = true;

                    m_logDeviceInfo.resource.logObj->initLogInfo ("Build/Log/Core", __FILE__);
                    LOG_WARNING (m_logDeviceInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                  << std::endl;
                }
                else {
                    m_logDeviceInfo.resource.logObj     = logObj;
                    m_logDeviceInfo.state.logObjCreated = false;
                }

                if (instanceObj == nullptr || phyDeviceObj == nullptr) {
                    LOG_ERROR (m_logDeviceInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_logDeviceInfo.resource.instanceObj    = instanceObj;
                m_logDeviceInfo.resource.phyDeviceObj   = phyDeviceObj;
            }

            void initLogDeviceInfo (void) {
                m_logDeviceInfo.meta.graphicsQueue = nullptr;
                m_logDeviceInfo.meta.presentQueue  = nullptr;
                m_logDeviceInfo.meta.transferQueue = nullptr;
                m_logDeviceInfo.resource.device    = nullptr;
            }

            VkQueue* getGraphicsQueue (void) {
                return &m_logDeviceInfo.meta.graphicsQueue;
            }

            VkQueue* getPresentQueue (void) {
                return &m_logDeviceInfo.meta.presentQueue;
            }

            VkQueue* getTransferQueue (void) {
                return &m_logDeviceInfo.meta.transferQueue;
            }

            VkDevice* getLogDevice (void) {
                return &m_logDeviceInfo.resource.device;
            }

            void createLogDevice (void) {
                auto validationLayers          = m_logDeviceInfo.resource.instanceObj->getValidationLayers();
                bool validationLayersDisabled  = m_logDeviceInfo.resource.instanceObj->isValidationLayersDisabled();
                bool validationLayersSupported = m_logDeviceInfo.resource.instanceObj->isValidationLayersSupported();
                auto deviceExtensions          = m_logDeviceInfo.resource.phyDeviceObj->getDeviceExtensions();
                auto queueCreateInfos          = std::vector <VkDeviceQueueCreateInfo> {};
                auto queueFamilyIndices        = std::set <uint32_t> {
                    m_logDeviceInfo.resource.phyDeviceObj->getGraphicsQueueFamilyIdx(),
                    m_logDeviceInfo.resource.phyDeviceObj->getPresentQueueFamilyIdx(),
                    m_logDeviceInfo.resource.phyDeviceObj->getTransferQueueFamilyIdx()
                };
                /* Assign priorities to queues to influence the scheduling of command buffer execution using floating
                 * point numbers between 0.0 and 1.0. This is required even if there is only a single queue
                */
                float queuePriority = 1.0f;
                for (auto const& queueFamilyIdx: queueFamilyIndices) {
                    VkDeviceQueueCreateInfo createInfo;
                    createInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    createInfo.pNext            = nullptr;
                    createInfo.flags            = 0;
                    createInfo.queueFamilyIndex = queueFamilyIdx;
                    createInfo.queueCount       = 1;
                    createInfo.pQueuePriorities = &queuePriority;

                    queueCreateInfos.push_back (createInfo);
                }

                VkDeviceCreateInfo createInfo;
                createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                createInfo.flags                   = 0;
                createInfo.queueCreateInfoCount    = static_cast <uint32_t> (queueCreateInfos.size());
                createInfo.pQueueCreateInfos       = queueCreateInfos.data();
                /* If we are using pNext, then pEnabledFeatures will have to be null as required by the spec */
                createInfo.pEnabledFeatures        = nullptr;

                createInfo.enabledExtensionCount   = static_cast <uint32_t> (deviceExtensions.size());
                createInfo.ppEnabledExtensionNames = deviceExtensions.data();
                /* Previous implementations of vulkan made a distinction between instance and device specific validation
                 * layers, but this is no longer the case. That means that the enabledLayerCount and ppEnabledLayerNames
                 * fields of VkDeviceCreateInfo are ignored by up-to-date implementations. However, it is still a good
                 * idea to set them anyway to be compatible with older implementations
                */
                createInfo.enabledLayerCount       = 0;
                createInfo.ppEnabledLayerNames     = nullptr;

                if (!validationLayersDisabled && !validationLayersSupported)
                    LOG_WARNING (m_logDeviceInfo.resource.logObj) << "Required validation layers not available"
                                                                  << std::endl;
                else if (!validationLayersDisabled) {
                    createInfo.enabledLayerCount   = static_cast <uint32_t> (validationLayers.size());
                    createInfo.ppEnabledLayerNames = validationLayers.data();
                }

                /* Specify set of device features that we'll be using */
                VkPhysicalDeviceFeatures features{};
                features.samplerAnisotropy         = VK_TRUE;
                features.sampleRateShading         = VK_TRUE;
                features.fillModeNonSolid          = VK_TRUE;

                VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
                descriptorIndexingFeatures.sType   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
                descriptorIndexingFeatures.pNext   = nullptr;
                descriptorIndexingFeatures.runtimeDescriptorArray                       = VK_TRUE;
                descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;

                VkPhysicalDeviceFeatures2 features2;
                features2.sType                    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                features2.pNext                    = &descriptorIndexingFeatures;
                features2.features                 = features;

                createInfo.pNext                   = &features2;

                auto result = vkCreateDevice (*m_logDeviceInfo.resource.phyDeviceObj->getPhyDevice(),
                                               &createInfo,
                                               nullptr,
                                               &m_logDeviceInfo.resource.device);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_logDeviceInfo.resource.logObj) << "[?] Log device"
                                                                << " "
                                                                << "[" << string_VkResult (result) << "]"
                                                                << std::endl;
                    throw std::runtime_error ("[?] Log device");
                }
                LOG_INFO (m_logDeviceInfo.resource.logObj)      << "[O] Log device"
                                                                << std::endl;

                /* Create queues */
                vkGetDeviceQueue (m_logDeviceInfo.resource.device,
                                  m_logDeviceInfo.resource.phyDeviceObj->getGraphicsQueueFamilyIdx(),
                                  0,
                                  &m_logDeviceInfo.meta.graphicsQueue);
                vkGetDeviceQueue (m_logDeviceInfo.resource.device,
                                  m_logDeviceInfo.resource.phyDeviceObj->getPresentQueueFamilyIdx(),
                                  0,
                                  &m_logDeviceInfo.meta.presentQueue);
                vkGetDeviceQueue (m_logDeviceInfo.resource.device,
                                  m_logDeviceInfo.resource.phyDeviceObj->getTransferQueueFamilyIdx(),
                                  0,
                                  &m_logDeviceInfo.meta.transferQueue);
            }

            void destroyLogDevice (void) {
                vkDestroyDevice (m_logDeviceInfo.resource.device, nullptr);
                LOG_INFO (m_logDeviceInfo.resource.logObj) << "[X] Log device"
                                                           << std::endl;
            }

            ~VKLogDevice (void) {
                if (m_logDeviceInfo.state.logObjCreated)
                    delete m_logDeviceInfo.resource.logObj;
            }
    };
}   // namespace Core