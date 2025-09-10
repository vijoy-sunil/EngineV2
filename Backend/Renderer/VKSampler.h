#pragma once
#include "../Common.h"
#include "../Collection/CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"
#include "VKLogDevice.h"

namespace Renderer {
    class VKSampler: public Collection::CNTypeInstanceBase {
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

            void createSampler (void) {
                auto& meta                         = m_samplerInfo.meta;
                auto& anisotropyDisabled           = m_samplerInfo.state.anisotropyDisabled;
                auto& resource                     = m_samplerInfo.resource;

                VkSamplerCreateInfo createInfo;
                createInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                createInfo.pNext                   = nullptr;
                createInfo.flags                   = 0;
                createInfo.mipLodBias              = meta.mipLodBias;
                createInfo.minLod                  = meta.minLod;
                createInfo.maxLod                  = meta.maxLod;
                if (anisotropyDisabled)
                    createInfo.maxAnisotropy       = 1.0f;
                else
                    createInfo.maxAnisotropy       = meta.maxAnisotropy;

                createInfo.magFilter               = meta.filter;
                createInfo.minFilter               = meta.filter;
                createInfo.addressModeU            = meta.addressMode;
                createInfo.addressModeV            = meta.addressMode;
                createInfo.addressModeW            = meta.addressMode;
                createInfo.mipmapMode              = meta.mipMapMode;
                createInfo.borderColor             = meta.borderColor;
                createInfo.anisotropyEnable        = !anisotropyDisabled;

                createInfo.unnormalizedCoordinates = VK_FALSE;
                createInfo.compareEnable           = VK_FALSE;
                createInfo.compareOp               = VK_COMPARE_OP_NEVER;

                auto result = vkCreateSampler (*resource.logDeviceObj->getLogDevice(),
                                                &createInfo,
                                                nullptr,
                                                &resource.sampler);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (resource.logObj) << "[?] Sampler"
                                                << " "
                                                << "[" << string_VkResult (result) << "]"
                                                << std::endl;
                    throw std::runtime_error ("[?] Sampler");
                }
                LOG_INFO (resource.logObj)      << "[O] Sampler"
                                                << std::endl;
            }

            void destroySampler (void) {
                auto& resource = m_samplerInfo.resource;
                vkDestroySampler (*resource.logDeviceObj->getLogDevice(),
                                   resource.sampler,
                                   nullptr);
                LOG_INFO (resource.logObj) << "[X] Sampler"
                                           << std::endl;
            }

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

                auto& meta                             = m_samplerInfo.meta;
                meta.mipLodBias                        = mipLodBias;
                meta.minLod                            = minLod;
                meta.maxLod                            = maxLod;
                meta.maxAnisotropy                     = maxAnisotropy;
                meta.filter                            = filter;
                meta.addressMode                       = addressMode;
                meta.mipMapMode                        = mipMapMode;
                meta.borderColor                       = borderColor;
                m_samplerInfo.state.anisotropyDisabled = anisotropyDisabled;
                m_samplerInfo.resource.sampler         = nullptr;
            }

            VkSampler* getSampler (void) {
                return &m_samplerInfo.resource.sampler;
            }

            void onAttach (void) override {
                createSampler();
            }

            void onDetach (void) override {
                destroySampler();
            }

            void onUpdate (void) override {
                /* Do nothing */
            }

            ~VKSampler (void) {
                if (m_samplerInfo.state.logObjCreated)
                    delete m_samplerInfo.resource.logObj;
            }
    };
}   // namespace Renderer