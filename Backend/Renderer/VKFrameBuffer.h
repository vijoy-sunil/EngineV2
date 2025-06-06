#pragma once
#include "../Common.h"
#include "../Collection/CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"
#include "VKLogDevice.h"
#include "VKRenderPass.h"

namespace Renderer {
    class VKFrameBuffer: public Collection::CNTypeInstanceBase {
        private:
            struct FrameBufferInfo {
                struct Meta {
                    uint32_t width;
                    uint32_t height;
                    uint32_t layersCount;
                    std::vector <VkImageView> attachments;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKLogDevice* logDeviceObj;
                    VKRenderPass* renderPassObj;
                    VkFramebuffer buffer;
                } resource;
            } m_frameBufferInfo;

            /* Render passes operate in conjunction with frame buffers. Frame buffers represent a collection of specific
             * memory attachments that a render pass instance uses. In other words, a frame buffer binds a VkImageView
             * with an attachment, and the frame buffer together with the render pass defines the render target
            */
            void createFrameBuffer (void) {
                VkFramebufferCreateInfo createInfo;
                createInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                createInfo.pNext           = nullptr;
                createInfo.flags           = 0;
                createInfo.width           = m_frameBufferInfo.meta.width;
                createInfo.height          = m_frameBufferInfo.meta.height;
                createInfo.layers          = m_frameBufferInfo.meta.layersCount;
                createInfo.attachmentCount = static_cast <uint32_t> (m_frameBufferInfo.meta.attachments.size());
                createInfo.pAttachments    = m_frameBufferInfo.meta.attachments.data();
                /* Note that, you can only use a frame buffer with the render passes that it is compatible with, which
                 * roughly means that they use the same number and type of attachments
                */
                createInfo.renderPass      = *m_frameBufferInfo.resource.renderPassObj->getRenderPass();

                auto result = vkCreateFramebuffer (*m_frameBufferInfo.resource.logDeviceObj->getLogDevice(),
                                                    &createInfo,
                                                    nullptr,
                                                    &m_frameBufferInfo.resource.buffer);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_frameBufferInfo.resource.logObj) << "[?] Frame buffer"
                                                                  << " "
                                                                  << "[" << string_VkResult (result) << "]"
                                                                  << std::endl;
                    throw std::runtime_error ("[?] Frame buffer");
                }
                LOG_INFO (m_frameBufferInfo.resource.logObj)      << "[O] Frame buffer"
                                                                  << std::endl;
            }

            void destroyFrameBuffer (void) {
                vkDestroyFramebuffer (*m_frameBufferInfo.resource.logDeviceObj->getLogDevice(),
                                       m_frameBufferInfo.resource.buffer,
                                       nullptr);
                LOG_INFO (m_frameBufferInfo.resource.logObj) << "[X] Frame buffer"
                                                             << std::endl;
            }

        public:
            VKFrameBuffer (Log::LGImpl*  logObj,
                           VKLogDevice*  logDeviceObj,
                           VKRenderPass* renderPassObj) {

                m_frameBufferInfo = {};

                if (logObj == nullptr) {
                    m_frameBufferInfo.resource.logObj     = new Log::LGImpl();
                    m_frameBufferInfo.state.logObjCreated = true;

                    m_frameBufferInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
                    LOG_WARNING (m_frameBufferInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                    << std::endl;
                }
                else {
                    m_frameBufferInfo.resource.logObj     = logObj;
                    m_frameBufferInfo.state.logObjCreated = false;
                }

                if (logDeviceObj == nullptr || renderPassObj == nullptr) {
                    LOG_ERROR (m_frameBufferInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                  << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_frameBufferInfo.resource.logDeviceObj   = logDeviceObj;
                m_frameBufferInfo.resource.renderPassObj  = renderPassObj;
            }

            void initFrameBufferInfo (const uint32_t width,
                                      const uint32_t height,
                                      const uint32_t layersCount,
                                      const std::vector <VkImageView> attachments) {

                m_frameBufferInfo.meta.width       = width;
                m_frameBufferInfo.meta.height      = height;
                m_frameBufferInfo.meta.layersCount = layersCount;
                m_frameBufferInfo.meta.attachments = attachments;
                m_frameBufferInfo.resource.buffer  = nullptr;
            }

            VkFramebuffer* getFrameBuffer (void) {
                return &m_frameBufferInfo.resource.buffer;
            }

            void onAttach (void) override {
                createFrameBuffer();
            }

            void onDetach (void) override {
                destroyFrameBuffer();
            }

            void onUpdate (void) override {
                /* Do nothing */
            }

            ~VKFrameBuffer (void) {
                if (m_frameBufferInfo.state.logObjCreated)
                    delete m_frameBufferInfo.resource.logObj;
            }
    };
}   // namespace Renderer