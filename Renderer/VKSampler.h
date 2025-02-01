#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vk_enum_string_helper.h>
#include "../Backend/Layer/LYInstanceBase.h"
#include "../Backend/Log/LGImpl.h"
#include "VKLogDevice.h"

namespace Renderer {
    class VKSampler: public Layer::LYInstanceBase {
        private:
            struct SamplerInfo {
                struct Meta {
                    float mipLodBias;
                    float minLod;
                    float maxLod;
                    float maxAnisotropy;
                    VkFilter filter;
                    VkSamplerAddressMode addressMode;
                    VkSamplerMipmapMode mipMapMode;
                    VkBorderColor borderColor;
                } meta;

                struct State {
                    VkBool32 anisotropyDisabled;
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKLogDevice* logDeviceObj;
                    VkSampler sampler;
                } resource;
            } m_samplerInfo;

        public:
            VKSampler (Log::LGImpl* logObj,
                       VKLogDevice* logDeviceObj) {

                m_samplerInfo = {};

                if (logObj == nullptr) {
                    m_samplerInfo.resource.logObj     = new Log::LGImpl();
                    m_samplerInfo.state.logObjCreated = true;

                    m_samplerInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
                    LOG_WARNING (m_samplerInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                << std::endl;
                }
                else {
                    m_samplerInfo.resource.logObj     = logObj;
                    m_samplerInfo.state.logObjCreated = false;
                }

                if (logDeviceObj == nullptr) {
                    LOG_ERROR (m_samplerInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                              << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_samplerInfo.resource.logDeviceObj   = logDeviceObj;
            }

            void initSamplerInfo (const float mipLodBias,
                                  const float minLod,
                                  const float maxLod,
                                  const float maxAnisotropy,
                                  const VkFilter filter,
                                  const VkSamplerAddressMode addressMode,
                                  const VkSamplerMipmapMode mipMapMode,
                                  const VkBorderColor borderColor,
                                  const VkBool32 anisotropyDisabled) {

                m_samplerInfo.meta.mipLodBias          = mipLodBias;
                m_samplerInfo.meta.minLod              = minLod;
                m_samplerInfo.meta.maxLod              = maxLod;
                m_samplerInfo.meta.maxAnisotropy       = maxAnisotropy;
                m_samplerInfo.meta.filter              = filter;
                m_samplerInfo.meta.addressMode         = addressMode;
                m_samplerInfo.meta.mipMapMode          = mipMapMode;
                m_samplerInfo.meta.borderColor         = borderColor;
                m_samplerInfo.state.anisotropyDisabled = anisotropyDisabled;
                m_samplerInfo.resource.sampler         = nullptr;
            }

            VkSampler* getSampler (void) {
                return &m_samplerInfo.resource.sampler;
            }

            void createSampler (void) {
                VkSamplerCreateInfo createInfo;
                createInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                createInfo.pNext                   = nullptr;
                createInfo.flags                   = 0;
                createInfo.mipLodBias              = m_samplerInfo.meta.mipLodBias;
                createInfo.minLod                  = m_samplerInfo.meta.minLod;
                createInfo.maxLod                  = m_samplerInfo.meta.maxLod;
                if (m_samplerInfo.state.anisotropyDisabled)
                    createInfo.maxAnisotropy       = 1.0f;
                else
                    createInfo.maxAnisotropy       = m_samplerInfo.meta.maxAnisotropy;

                createInfo.magFilter               = m_samplerInfo.meta.filter;
                createInfo.minFilter               = m_samplerInfo.meta.filter;
                createInfo.addressModeU            = m_samplerInfo.meta.addressMode;
                createInfo.addressModeV            = m_samplerInfo.meta.addressMode;
                createInfo.addressModeW            = m_samplerInfo.meta.addressMode;
                createInfo.mipmapMode              = m_samplerInfo.meta.mipMapMode;
                createInfo.borderColor             = m_samplerInfo.meta.borderColor;
                createInfo.anisotropyEnable        = !m_samplerInfo.state.anisotropyDisabled;

                createInfo.unnormalizedCoordinates = VK_FALSE;
                createInfo.compareEnable           = VK_FALSE;
                createInfo.compareOp               = VK_COMPARE_OP_ALWAYS;

                auto result = vkCreateSampler (*m_samplerInfo.resource.logDeviceObj->getLogDevice(),
                                                &createInfo,
                                                nullptr,
                                                &m_samplerInfo.resource.sampler);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_samplerInfo.resource.logObj) << "[?] Sampler"
                                                              << " "
                                                              << "[" << string_VkResult (result) << "]"
                                                              << std::endl;
                    throw std::runtime_error ("[?] Sampler");
                }
                LOG_INFO (m_samplerInfo.resource.logObj)      << "[O] Sampler"
                                                              << std::endl;
            }

            void destroySampler (void) {
                vkDestroySampler (*m_samplerInfo.resource.logDeviceObj->getLogDevice(),
                                   m_samplerInfo.resource.sampler,
                                   nullptr);
                LOG_INFO (m_samplerInfo.resource.logObj) << "[X] Sampler"
                                                         << std::endl;
            }

            ~VKSampler (void) {
                if (m_samplerInfo.state.logObjCreated)
                    delete m_samplerInfo.resource.logObj;
            }
    };
}   // namespace Renderer