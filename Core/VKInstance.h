#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <set>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include "../Backend/Layer/LYInstanceBase.h"
#include "../Backend/Log/LGImpl.h"

namespace Core {
    class VKInstance: public Layer::LYInstanceBase {
        private:
            struct InstanceInfo {
                struct Meta {
                    std::vector <const char*> extensions;
                    std::vector <const char*> validationLayers;
                } meta;

                struct State {
                    bool extensionsSupported;
                    bool validationLayersDisabled;
                    bool validationLayersSupported;
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VkInstance instance;
                    VkDebugUtilsMessengerEXT debugUtilsMessenger;
                } resource;
            } m_instanceInfo;

            static Log::LGImpl* m_validationLogObj;
            static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback (
                                                 const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                 const VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                 const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
                                                 void* userData) {
                /* Suppress unused parameter warning */
                static_cast <void> (userData);
                LOG_WARNING (m_validationLogObj) << "Msg severity "
                                                 << "["
                                                 << string_VkDebugUtilsMessageSeverityFlagBitsEXT (messageSeverity)
                                                 << "]"
                                                 << std::endl;
                LOG_WARNING (m_validationLogObj) << "Msg type "
                                                 << "["
                                                 << string_VkDebugUtilsMessageTypeFlagsEXT (messageType)
                                                 << "]"
                                                 << std::endl;
                LOG_WARNING (m_validationLogObj) << "Msg content "
                                                 << "["
                                                 << callbackData->pMessage
                                                 << "]"
                                                 << std::endl;
                LOG_WARNING (m_validationLogObj) << LINE_BREAK
                                                 << std::endl;
                /* The boolean indicates if the vulkan call that triggered the validation layer message should be aborted.
                 * If true, then the call is aborted with the VK_ERROR_VALIDATION_FAILED_EXT error
                */
                return VK_FALSE;
            }

            void populateDebugUtilsMessengerCreateInfo (VkDebugUtilsMessengerCreateInfoEXT* createInfo) {
                createInfo->sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                createInfo->pNext           = nullptr;
                createInfo->flags           = 0;
                createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT   |
                                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT   |
                                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                createInfo->messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT       |
                                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT    |
                                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                createInfo->pfnUserCallback = debugCallback;
                createInfo->pUserData       = nullptr;
            }

            VkResult createDebugUtilsMessengerEXT (const VkInstance instance,
                                                   const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
                                                   const VkAllocationCallbacks* allocator,
                                                   VkDebugUtilsMessengerEXT* debugUtilsMessenger) {

                auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr (instance,
                                                                 "vkCreateDebugUtilsMessengerEXT");
                if (func != nullptr)
                    return func (instance, createInfo, allocator, debugUtilsMessenger);
                else
                    return VK_ERROR_EXTENSION_NOT_PRESENT;
            }

            void destroyDebugUtilsMessengerEXT (const VkInstance instance,
                                                const VkAllocationCallbacks* allocator,
                                                const VkDebugUtilsMessengerEXT* debugUtilsMessenger) {

                auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr (instance,
                                                                  "vkDestroyDebugUtilsMessengerEXT");
                if (func != nullptr)
                    func (instance, *debugUtilsMessenger, allocator);
            }

            void createDebugUtilsMessenger (void) {
                if (isValidationLayersDisabled())
                    return;

                VkDebugUtilsMessengerCreateInfoEXT createInfo;
                populateDebugUtilsMessengerCreateInfo (&createInfo);

                auto result = createDebugUtilsMessengerEXT (m_instanceInfo.resource.instance,
                                                            &createInfo,
                                                            nullptr,
                                                            &m_instanceInfo.resource.debugUtilsMessenger);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_instanceInfo.resource.logObj) << "[?] Debug utils messenger"
                                                               << " "
                                                               << "[" << string_VkResult (result) << "]"
                                                               << std::endl;
                    throw std::runtime_error ("[?] Debug utils messenger");
                }
                LOG_INFO (m_instanceInfo.resource.logObj) << "[O] Debug utils messenger"
                                                          << std::endl;
            }

            void destroyDebugUtilsMessenger (void) {
                if (isValidationLayersDisabled())
                    return;

                destroyDebugUtilsMessengerEXT (m_instanceInfo.resource.instance,
                                               nullptr,
                                               &m_instanceInfo.resource.debugUtilsMessenger);

                LOG_INFO (m_instanceInfo.resource.logObj) << "[X] Debug utils messenger"
                                                          << std::endl;
            }

            void populateInstanceExtensions (void) {
                uint32_t glfwExtensionsCount = 0;
                /* Since Vulkan is a platform agnostic API, it can not interface directly with the window system on its
                 * own. To establish the connection between Vulkan and the window system to present results to the
                 * screen, we need to use the WSI (Window System Integration) extensions (ex: VK_KHR_surface) (included
                 * in glfw extensions)
                */
                glfwInit();
                auto glfwExtensions          = glfwGetRequiredInstanceExtensions (&glfwExtensionsCount);
                auto& extensions             = m_instanceInfo.meta.extensions;

                for (uint32_t i = 0; i < glfwExtensionsCount; i++)
                    extensions.emplace_back (glfwExtensions[i]);
#if __APPLE__
                extensions.emplace_back (VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
                extensions.emplace_back (VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif  // __APPLE__
                /* The validation layers will print debug messages to the standard output by default, but we can also
                 * handle them ourselves by providing an explicit callback in our program. Set up a debug utils messenger
                 * extension with a callback using the VK_EXT_debug_utils extension
                */
                if (!isValidationLayersDisabled())
                    extensions.emplace_back (VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            bool isInstanceExtensionsSupportedEXT (void) {
                uint32_t extensionsCount = 0;
                vkEnumerateInstanceExtensionProperties (nullptr, &extensionsCount, nullptr);
                std::vector <VkExtensionProperties> availableExtensions (extensionsCount);
                vkEnumerateInstanceExtensionProperties (nullptr, &extensionsCount, availableExtensions.data());

                LOG_INFO (m_instanceInfo.resource.logObj)     << "Available instance extensions"
                                                              << std::endl;
                for (auto const& extension: availableExtensions)
                    LOG_INFO (m_instanceInfo.resource.logObj) << "[" << extension.extensionName << "]"
                                                              << " "
                                                              << "[" << extension.specVersion   << "]"
                                                              << std::endl;
                LOG_INFO (m_instanceInfo.resource.logObj)     << LINE_BREAK
                                                              << std::endl;

                LOG_INFO (m_instanceInfo.resource.logObj)     << "Required instance extensions"
                                                              << std::endl;
                for (auto const& extension: m_instanceInfo.meta.extensions)
                    LOG_INFO (m_instanceInfo.resource.logObj) << "[" << extension << "]"
                                                              << std::endl;
                LOG_INFO (m_instanceInfo.resource.logObj)     << LINE_BREAK
                                                              << std::endl;

                std::set <std::string> requiredExtensions (m_instanceInfo.meta.extensions.begin(),
                                                           m_instanceInfo.meta.extensions.end());
                for (auto const& extension: availableExtensions)
                    requiredExtensions.erase (extension.extensionName);
                return requiredExtensions.empty();
            }

            bool isValidationLayersSupportedEXT (void) {
                uint32_t layersCount = 0;
                vkEnumerateInstanceLayerProperties (&layersCount, nullptr);
                std::vector <VkLayerProperties> availableLayers (layersCount);
                vkEnumerateInstanceLayerProperties (&layersCount, availableLayers.data());

                LOG_INFO (m_instanceInfo.resource.logObj)     << "Available validation layers"
                                                              << std::endl;
                for (auto const& layer: availableLayers)
                    LOG_INFO (m_instanceInfo.resource.logObj) << "[" << layer.layerName   << "]"
                                                              << " "
                                                              << "[" << layer.specVersion << "]"
                                                              << std::endl;
                LOG_INFO (m_instanceInfo.resource.logObj)     << LINE_BREAK
                                                              << std::endl;

                LOG_INFO (m_instanceInfo.resource.logObj)     << "Required validation layers"
                                                              << std::endl;
                for (auto const& layer: m_instanceInfo.meta.validationLayers)
                    LOG_INFO (m_instanceInfo.resource.logObj) << "[" << layer << "]"
                                                              << std::endl;
                LOG_INFO (m_instanceInfo.resource.logObj)     << LINE_BREAK
                                                              << std::endl;

                std::set <std::string> requiredLayers (m_instanceInfo.meta.validationLayers.begin(),
                                                       m_instanceInfo.meta.validationLayers.end());
                for (auto const& layer: availableLayers)
                    requiredLayers.erase (layer.layerName);
                return requiredLayers.empty();
            }

        public:
            VKInstance (Log::LGImpl* logObj,
                        const std::vector <const char*> validationLayers,
                        const bool validationLayersDisabled = false) {

                if (logObj == nullptr) {
                    m_instanceInfo.resource.logObj             = new Log::LGImpl();
                    LOG_WARNING (m_instanceInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                 << std::endl;
                    m_instanceInfo.state.logObjCreated         = true;
                }
                else {
                    m_instanceInfo.resource.logObj             = logObj;
                    m_instanceInfo.state.logObjCreated         = false;
                }

                m_validationLogObj = new Log::LGImpl ("Build/Log/Core",       "validationLog.txt");
                m_validationLogObj->updateLogConfig  (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_NONE);
                m_validationLogObj->updateLogConfig  (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_CONSOLE |
                                                                              Log::LOG_SINK_FILE);
                m_validationLogObj->updateLogConfig  (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_NONE);

                populateInstanceExtensions();
                m_instanceInfo.meta.validationLayers           = validationLayers;
                m_instanceInfo.state.extensionsSupported       = isInstanceExtensionsSupportedEXT();
                m_instanceInfo.state.validationLayersDisabled  = validationLayersDisabled;
                m_instanceInfo.state.validationLayersSupported = isValidationLayersSupportedEXT();
                m_instanceInfo.resource.instance               = nullptr;
                m_instanceInfo.resource.debugUtilsMessenger    = nullptr;
            }

            bool isInstanceExtensionsSupported (void) {
                return m_instanceInfo.state.extensionsSupported;
            }

            bool isValidationLayersDisabled (void) {
                return m_instanceInfo.state.validationLayersDisabled;
            }

            bool isValidationLayersSupported (void) {
                return m_instanceInfo.state.validationLayersSupported;
            }

            void toggleValidationLayers (const bool val) {
                m_instanceInfo.state.validationLayersDisabled = !val;
            }

            std::vector <const char*> getValidationLayers (void) {
                return m_instanceInfo.meta.validationLayers;
            }

            VkInstance* getInstance (void) {
                return &m_instanceInfo.resource.instance;
            }

            void createInstance (void) {
                /* The vkCreateDebugUtilsMessengerEXT call requires a valid instance to have been created and
                 * vkDestroyDebugUtilsMessengerEXT must be called before the instance is destroyed. This currently leaves
                 * us unable to debug any issues in the vkCreateInstance and vkDestroyInstance calls. However, you'll see
                 * that there is a way to create a separate debug utils messenger specifically for those two function
                 * calls. It requires you to simply pass a pointer to a VkDebugUtilsMessengerCreateInfoEXT struct in the
                 * pNext extension field of VkInstanceCreateInfo
                */
                VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo;
                auto validationLayers              = getValidationLayers();

                VkApplicationInfo appInfo;
                appInfo.sType                      = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                appInfo.pNext                      = nullptr;
                appInfo.pApplicationName           = "SANDBOX_ENGINE";
                appInfo.applicationVersion         = VK_MAKE_VERSION(1, 0, 0);
                appInfo.pEngineName                = "NO ENGINE";
                appInfo.engineVersion              = VK_MAKE_VERSION(0, 0, 0);
                appInfo.apiVersion                 = VK_API_VERSION_1_0;

                VkInstanceCreateInfo createInfo;
                createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                createInfo.pApplicationInfo        = &appInfo;
                createInfo.flags                   = 0;

                if (!isInstanceExtensionsSupported()) {
                    LOG_ERROR (m_instanceInfo.resource.logObj) << "Required instance extensions not available"
                                                               << std::endl;
                    throw std::runtime_error ("Required instance extensions not available");
                }
                createInfo.enabledExtensionCount   = static_cast <uint32_t> (m_instanceInfo.meta.extensions.size());
                createInfo.ppEnabledExtensionNames = m_instanceInfo.meta.extensions.data();

                /* Set up validation layers */
                createInfo.enabledLayerCount       = 0;
                createInfo.ppEnabledLayerNames     = nullptr;
                createInfo.pNext                   = nullptr;

                if (!isValidationLayersDisabled() && !isValidationLayersSupported())
                    LOG_WARNING (m_instanceInfo.resource.logObj) << "Required validation layers not available"
                                                                 << std::endl;
                else if (!isValidationLayersDisabled()) {
                    populateDebugUtilsMessengerCreateInfo (&debugUtilsCreateInfo);

                    createInfo.enabledLayerCount   = static_cast <uint32_t> (validationLayers.size());
                    createInfo.ppEnabledLayerNames = validationLayers.data();
                    createInfo.pNext               = static_cast <VkDebugUtilsMessengerCreateInfoEXT*>
                                                     (&debugUtilsCreateInfo);
                }
#if __APPLE__
                createInfo.flags                  |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif  // __APPLE__

                auto result = vkCreateInstance (&createInfo, nullptr, &m_instanceInfo.resource.instance);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_instanceInfo.resource.logObj) << "[?] Instance"
                                                               << " "
                                                               << "[" << string_VkResult (result) << "]"
                                                               << std::endl;
                    throw std::runtime_error ("[?] Instance");
                }

                LOG_INFO (m_instanceInfo.resource.logObj) << "[O] Instance"
                                                          << std::endl;
                createDebugUtilsMessenger();
            }

            void destroyInstance (void) {
                destroyDebugUtilsMessenger();
                /* Note that, the vulkan instance should be only destroyed right before the program exits, all of the
                 * other vulkan resources that we create should be cleaned up before the instance is destroyed
                */
                vkDestroyInstance (m_instanceInfo.resource.instance, nullptr);
                LOG_INFO (m_instanceInfo.resource.logObj) << "[X] Instance"
                                                          << std::endl;

                if (m_instanceInfo.state.logObjCreated)
                    delete m_instanceInfo.resource.logObj;
                delete m_validationLogObj;
            }
    };
    Log::LGImpl* VKInstance::m_validationLogObj = nullptr;
}   // namespace Core