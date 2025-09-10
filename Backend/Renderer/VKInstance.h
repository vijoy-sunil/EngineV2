#pragma once
#include "../Common.h"
#include "../Collection/CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"

namespace Renderer {
    class VKInstance: public Collection::CNTypeInstanceBase {
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
                                                  const VkDebugUtilsMessageTypeFlagsEXT messageTypes,
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
                                                 << string_VkDebugUtilsMessageTypeFlagsEXT (messageTypes)
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

            VkDebugUtilsMessengerCreateInfoEXT createDebugUtilsMessengerEXT (void) {
                VkDebugUtilsMessengerCreateInfoEXT createInfo;
                createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                createInfo.pNext           = nullptr;
                createInfo.flags           = 0;
                createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                createInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
                                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
                                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                createInfo.pfnUserCallback = debugCallback;
                createInfo.pUserData       = nullptr;

                return createInfo;
            }

            void createDebugUtilsMessenger (void) {
                if (isValidationLayersDisabled())
                    return;

                auto& resource  = m_instanceInfo.resource;
                auto createInfo = createDebugUtilsMessengerEXT();
                auto binding    = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr (
                    resource.instance,
                    "vkCreateDebugUtilsMessengerEXT"
                );
                auto result     = binding != nullptr ? binding (
                    resource.instance,
                    &createInfo,
                    nullptr,
                    &resource.debugUtilsMessenger
                ):  VK_ERROR_EXTENSION_NOT_PRESENT;

                if (result != VK_SUCCESS) {
                    LOG_ERROR (resource.logObj) << "[?] Debug utils messenger"
                                                << " "
                                                << "[" << string_VkResult (result) << "]"
                                                << std::endl;
                    throw std::runtime_error ("[?] Debug utils messenger");
                }
                LOG_INFO (resource.logObj)      << "[O] Debug utils messenger"
                                                << std::endl;
            }

            void destroyDebugUtilsMessenger (void) {
                if (isValidationLayersDisabled())
                    return;

                auto& resource = m_instanceInfo.resource;
                auto binding   = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr (
                    resource.instance,
                    "vkDestroyDebugUtilsMessengerEXT"
                );
                if (binding != nullptr) binding (
                    resource.instance,
                    resource.debugUtilsMessenger,
                    nullptr
                );
                LOG_INFO (resource.logObj) << "[X] Debug utils messenger"
                                           << std::endl;
            }

            std::vector <const char*> getInstanceExtensions (void) {
                auto extensions = std::vector <const char*> {};
                /* Since Vulkan is a platform agnostic API, it can not interface directly with the window system on its
                 * own. To establish the connection between Vulkan and the window system to present results to the
                 * screen, we need to use the WSI (Window System Integration) extensions (ex: VK_KHR_surface) (included
                 * in glfw extensions)
                */
                glfwInit();
                uint32_t glfwExtensionsCount = 0;
                auto glfwExtensions          = glfwGetRequiredInstanceExtensions (&glfwExtensionsCount);

                for (uint32_t i = 0; i < glfwExtensionsCount; i++)
                    extensions.emplace_back (glfwExtensions[i]);
#if __APPLE__
                extensions.emplace_back (VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
                extensions.emplace_back (VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif  // __APPLE__
                /* The validation layers will print debug messages to the standard output by default, but we can also
                 * handle them ourselves by providing an explicit callback in our program using the VK_EXT_debug_utils
                 * extension
                */
                if (!isValidationLayersDisabled())
                    extensions.emplace_back (VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

                return extensions;
            }

            bool isInstanceExtensionsSupportedEXT (void) {
                auto& extensions         = m_instanceInfo.meta.extensions;
                auto& logObj             = m_instanceInfo.resource.logObj;
                uint32_t extensionsCount = 0;

                vkEnumerateInstanceExtensionProperties (nullptr, &extensionsCount, nullptr);
                std::vector <VkExtensionProperties> availableExtensions (extensionsCount);
                vkEnumerateInstanceExtensionProperties (nullptr, &extensionsCount, availableExtensions.data());

                LOG_INFO (logObj)     << "Available instance extensions"
                                      << std::endl;
                for (auto const& extension: availableExtensions)
                    LOG_INFO (logObj) << "[" << extension.extensionName << "]"
                                      << " "
                                      << "[" << extension.specVersion   << "]"
                                      << std::endl;
                LOG_INFO (logObj)     << LINE_BREAK
                                      << std::endl;

                LOG_INFO (logObj)     << "Required instance extensions"
                                      << std::endl;
                for (auto const& extension: extensions)
                    LOG_INFO (logObj) << "[" << extension               << "]"
                                      << std::endl;
                LOG_INFO (logObj)     << LINE_BREAK
                                      << std::endl;

                std::set <std::string> requiredExtensions (extensions.begin(), extensions.end());
                for (auto const& extension: availableExtensions)
                    requiredExtensions.erase (extension.extensionName);
                return requiredExtensions.empty();
            }

            bool isInstanceExtensionsSupported (void) {
                return m_instanceInfo.state.extensionsSupported;
            }

            bool isValidationLayersSupportedEXT (void) {
                auto& validationLayers = m_instanceInfo.meta.validationLayers;
                auto& logObj           = m_instanceInfo.resource.logObj;
                uint32_t layersCount   = 0;

                vkEnumerateInstanceLayerProperties (&layersCount, nullptr);
                std::vector <VkLayerProperties> availableLayers (layersCount);
                vkEnumerateInstanceLayerProperties (&layersCount, availableLayers.data());

                LOG_INFO (logObj)     << "Available validation layers"
                                      << std::endl;
                for (auto const& layer: availableLayers)
                    LOG_INFO (logObj) << "[" << layer.layerName   << "]"
                                      << " "
                                      << "[" << layer.specVersion << "]"
                                      << std::endl;
                LOG_INFO (logObj)     << LINE_BREAK
                                      << std::endl;

                LOG_INFO (logObj)     << "Required validation layers"
                                      << std::endl;
                for (auto const& layer: validationLayers)
                    LOG_INFO (logObj) << "[" << layer             << "]"
                                      << std::endl;
                LOG_INFO (logObj)     << LINE_BREAK
                                      << std::endl;

                std::set <std::string> requiredLayers (validationLayers.begin(), validationLayers.end());
                for (auto const& layer: availableLayers)
                    requiredLayers.erase (layer.layerName);
                return requiredLayers.empty();
            }

            void createInstance (void) {
                auto& extensions                   = m_instanceInfo.meta.extensions;
                auto& resource                     = m_instanceInfo.resource;
                /* The vkCreateDebugUtilsMessengerEXT call requires a valid instance to have been created and
                 * vkDestroyDebugUtilsMessengerEXT must be called before the instance is destroyed. This currently leaves
                 * us unable to debug any issues in the vkCreateInstance and vkDestroyInstance calls. However, you'll see
                 * that there is a way to create a separate debug utils messenger specifically for those two function
                 * calls. It requires you to simply pass a pointer to a VkDebugUtilsMessengerCreateInfoEXT struct in the
                 * pNext extension field of VkInstanceCreateInfo
                */
                auto debugUtilsCreateInfo          = createDebugUtilsMessengerEXT();
                auto validationLayers              = getValidationLayers();

                VkApplicationInfo appInfo;
                appInfo.sType                      = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                appInfo.pNext                      = nullptr;
                appInfo.pApplicationName           = "Engine";
                appInfo.applicationVersion         = VK_MAKE_API_VERSION (2, 0, 0, 0);
                appInfo.pEngineName                = "Vulkan";
                appInfo.engineVersion              = VK_MAKE_API_VERSION (0, 0, 0, 0);
                appInfo.apiVersion                 = VK_MAKE_API_VERSION (0, 1, 3, 0);

                VkInstanceCreateInfo createInfo;
                createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                createInfo.pApplicationInfo        = &appInfo;
                createInfo.flags                   = 0;

                if (!isInstanceExtensionsSupported()) {
                    LOG_ERROR (resource.logObj) << "Required instance extensions not available"
                                                << std::endl;
                    throw std::runtime_error ("Required instance extensions not available");
                }
                createInfo.enabledExtensionCount   = static_cast <uint32_t> (extensions.size());
                createInfo.ppEnabledExtensionNames = extensions.data();

                /* Set up validation layers */
                createInfo.enabledLayerCount       = 0;
                createInfo.ppEnabledLayerNames     = nullptr;
                createInfo.pNext                   = nullptr;

                if (!isValidationLayersDisabled() && !isValidationLayersSupported())
                    LOG_WARNING (resource.logObj) << "Required validation layers not available"
                                                  << std::endl;
                else if (!isValidationLayersDisabled()) {
                    createInfo.enabledLayerCount   = static_cast <uint32_t> (validationLayers.size());
                    createInfo.ppEnabledLayerNames = validationLayers.data();
                    createInfo.pNext               = static_cast <VkDebugUtilsMessengerCreateInfoEXT*>
                                                     (&debugUtilsCreateInfo);
                }
#if __APPLE__
                createInfo.flags                  |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif  // __APPLE__

                auto result = vkCreateInstance (&createInfo,
                                                nullptr,
                                                &resource.instance);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (resource.logObj) << "[?] Instance"
                                                << " "
                                                << "[" << string_VkResult (result) << "]"
                                                << std::endl;
                    throw std::runtime_error ("[?] Instance");
                }
                LOG_INFO (resource.logObj)      << "[O] Instance"
                                                << std::endl;
                createDebugUtilsMessenger();
            }

            void destroyInstance (void) {
                auto& resource = m_instanceInfo.resource;
                destroyDebugUtilsMessenger();
                /* Note that, the vulkan instance should be only destroyed right before the program exits, all of the
                 * other vulkan resources that we create should be cleaned up before the instance is destroyed
                */
                vkDestroyInstance (resource.instance, nullptr);
                LOG_INFO (resource.logObj) << "[X] Instance"
                                           << std::endl;
            }

        public:
            VKInstance (Log::LGImpl* logObj) {
                m_instanceInfo = {};

                if (logObj == nullptr) {
                    m_instanceInfo.resource.logObj     = new Log::LGImpl();
                    m_instanceInfo.state.logObjCreated = true;

                    m_instanceInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
                    LOG_WARNING (m_instanceInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                 << std::endl;
                }
                else {
                    m_instanceInfo.resource.logObj     = logObj;
                    m_instanceInfo.state.logObjCreated = false;
                }

                m_validationLogObj = new Log::LGImpl();
                m_validationLogObj->initLogInfo     ("Build/Log/Renderer",    "Validation");
                m_validationLogObj->updateLogConfig (Log::LEVEL_TYPE_INFO,    Log::SINK_TYPE_NONE);
                m_validationLogObj->updateLogConfig (Log::LEVEL_TYPE_WARNING, Log::SINK_TYPE_CONSOLE | Log::SINK_TYPE_FILE);
                m_validationLogObj->updateLogConfig (Log::LEVEL_TYPE_ERROR,   Log::SINK_TYPE_NONE);
            }

            void initInstanceInfo (const std::vector <const char*> validationLayers,
                                   const bool validationLayersDisabled = false) {

                auto& meta                      = m_instanceInfo.meta;
                auto& state                     = m_instanceInfo.state;
                auto& resource                  = m_instanceInfo.resource;

                meta.extensions                 = getInstanceExtensions();
                meta.validationLayers           = validationLayers;

                state.extensionsSupported       = isInstanceExtensionsSupportedEXT();
                state.validationLayersDisabled  = validationLayersDisabled;
                state.validationLayersSupported = isValidationLayersSupportedEXT();

                resource.instance               = nullptr;
                resource.debugUtilsMessenger    = nullptr;
            }

            bool isValidationLayersDisabled (void) {
                return m_instanceInfo.state.validationLayersDisabled;
            }

            bool isValidationLayersSupported (void) {
                return m_instanceInfo.state.validationLayersSupported;
            }

            std::vector <const char*>& getValidationLayers (void) {
                return m_instanceInfo.meta.validationLayers;
            }

            VkInstance* getInstance (void) {
                return &m_instanceInfo.resource.instance;
            }

            void onAttach (void) override {
                createInstance();
            }

            void onDetach (void) override {
                destroyInstance();
            }

            void onUpdate (void) override {
                /* Do nothing */
            }

            ~VKInstance (void) {
                if (m_instanceInfo.state.logObjCreated)
                    delete m_instanceInfo.resource.logObj;
                delete m_validationLogObj;
            }
    };
    Log::LGImpl* VKInstance::m_validationLogObj = nullptr;
}   // namespace Renderer