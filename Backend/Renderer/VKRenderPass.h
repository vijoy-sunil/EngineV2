#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include "../Collection/CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"
#include "VKLogDevice.h"

namespace Renderer {
    class VKRenderPass: public Collection::CNTypeInstanceBase {
        private:
            struct RenderPassInfo {
                struct Meta {
                    std::vector <VkAttachmentDescription> attachments;
                    std::vector <VkSubpassDescription>    subPasses;
                    std::vector <VkSubpassDependency>     dependencies;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKLogDevice* logDeviceObj;
                    VkRenderPass renderPass;
                } resource;
            } m_renderPassInfo;

            void createRenderPass (void) {
                VkRenderPassCreateInfo createInfo;
                createInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                createInfo.pNext           = nullptr;
                createInfo.flags           = 0;
                createInfo.attachmentCount = static_cast <uint32_t> (m_renderPassInfo.meta.attachments.size());
                createInfo.pAttachments    = m_renderPassInfo.meta.attachments.data();
                createInfo.subpassCount    = static_cast <uint32_t> (m_renderPassInfo.meta.subPasses.size());
                createInfo.pSubpasses      = m_renderPassInfo.meta.subPasses.data();
                createInfo.dependencyCount = static_cast <uint32_t> (m_renderPassInfo.meta.dependencies.size());
                createInfo.pDependencies   = m_renderPassInfo.meta.dependencies.data();

                auto result = vkCreateRenderPass (*m_renderPassInfo.resource.logDeviceObj->getLogDevice(),
                                                   &createInfo,
                                                   nullptr,
                                                   &m_renderPassInfo.resource.renderPass);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_renderPassInfo.resource.logObj) << "[?] Render pass"
                                                                 << " "
                                                                 << "[" << string_VkResult (result) << "]"
                                                                 << std::endl;
                    throw std::runtime_error ("[?] Render pass");
                }
                LOG_INFO (m_renderPassInfo.resource.logObj)      << "[O] Render pass"
                                                                 << std::endl;
            }

            void destroyRenderPass (void) {
                vkDestroyRenderPass  (*m_renderPassInfo.resource.logDeviceObj->getLogDevice(),
                                       m_renderPassInfo.resource.renderPass,
                                       nullptr);
                LOG_INFO (m_renderPassInfo.resource.logObj) << "[X] Render pass"
                                                            << std::endl;
            }

        public:
            VKRenderPass (Log::LGImpl* logObj,
                          VKLogDevice* logDeviceObj) {

                m_renderPassInfo = {};

                if (logObj == nullptr) {
                    m_renderPassInfo.resource.logObj     = new Log::LGImpl();
                    m_renderPassInfo.state.logObjCreated = true;

                    m_renderPassInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
                    LOG_WARNING (m_renderPassInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                   << std::endl;
                }
                else {
                    m_renderPassInfo.resource.logObj     = logObj;
                    m_renderPassInfo.state.logObjCreated = false;
                }

                if (logDeviceObj == nullptr) {
                    LOG_ERROR (m_renderPassInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                                 << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_renderPassInfo.resource.logDeviceObj   = logDeviceObj;
            }

            void initRenderPassInfo (void) {
                m_renderPassInfo.meta.attachments    = {};
                m_renderPassInfo.meta.subPasses      = {};
                m_renderPassInfo.meta.dependencies   = {};
                m_renderPassInfo.resource.renderPass = nullptr;
            }

            void addRenderPassAttachment (const VkAttachmentDescriptionFlags flags,
                                          const VkFormat format,
                                          const VkSampleCountFlagBits samplesCount,
                                          const VkAttachmentLoadOp loadOp,
                                          const VkAttachmentStoreOp storeOp,
                                          const VkAttachmentLoadOp stencilLoadOp,
                                          const VkAttachmentStoreOp stencilStoreOp,
                                          const VkImageLayout initialImageLayout,
                                          const VkImageLayout finalImageLayout) {

                VkAttachmentDescription attachment;
                attachment.flags          = flags;
                attachment.format         = format;
                attachment.samples        = samplesCount;
                attachment.loadOp         = loadOp;
                attachment.storeOp        = storeOp;
                attachment.stencilLoadOp  = stencilLoadOp;
                attachment.stencilStoreOp = stencilStoreOp;
                attachment.initialLayout  = initialImageLayout;
                attachment.finalLayout    = finalImageLayout;

                m_renderPassInfo.meta.attachments.push_back (attachment);
            }

            /* The VkAttachmentReference does not reference the attachment object directly, it references the index in
             * the attachments array specified in VkRenderPassCreateInfo. This allows different sub passes to reference
             * the same attachment
             *
             * The attachment reference layout tells vulkan what layout to transition the image to at the beginning of
             * the sub pass for which this reference is defined. Or more to the point, it is the layout which the image
             * will be in for the duration of the sub pass. Note that, vulkan will automatically transition the attachment
             * to this layout when the sub pass is started
            */
            VkAttachmentReference createAttachmentReference (const uint32_t attachmentIdx,
                                                             const VkImageLayout imageLayout) {

                VkAttachmentReference attachmentReference;
                attachmentReference.attachment = attachmentIdx;
                attachmentReference.layout     = imageLayout;
                return attachmentReference;
            }

            void addSubPass (const std::vector <VkAttachmentReference>& inputAttachmentReferences,
                             const std::vector <VkAttachmentReference>& colorAttachmentReferences,
                             const VkAttachmentReference* depthStencilAttachmentReference,
                             const std::vector <VkAttachmentReference>& resolveAttachmentReferences) {

                VkSubpassDescription subPass;
                subPass.flags                   = 0;
                subPass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
                subPass.inputAttachmentCount    = static_cast <uint32_t> (inputAttachmentReferences.size());
                subPass.pInputAttachments       = inputAttachmentReferences.data();
                subPass.preserveAttachmentCount = 0;
                subPass.pPreserveAttachments    = nullptr;
                /* Note that, the index of the attachment in this array is directly referenced from the fragment shader
                 * with the layout (location = ?) out directive
                */
                subPass.colorAttachmentCount    = static_cast <uint32_t> (colorAttachmentReferences.size());
                subPass.pColorAttachments       = colorAttachmentReferences.data();
                subPass.pDepthStencilAttachment = depthStencilAttachmentReference;
                subPass.pResolveAttachments     = resolveAttachmentReferences.data();

                m_renderPassInfo.meta.subPasses.push_back (subPass);
            }

            void addSubPassDependency (const VkDependencyFlags flags,
                                       const uint32_t srcSubPassIdx,
                                       const uint32_t dstSubPassIdx,
                                       const VkPipelineStageFlags srcStageMask,
                                       const VkPipelineStageFlags dstStageMask,
                                       const VkAccessFlags srcAccessMask,
                                       const VkAccessFlags dstAccessMask) {

                VkSubpassDependency dependency;
                dependency.dependencyFlags = flags;
                /* srcSubpass is the index of the sub pass we're dependant on. If we wanted to depend on a sub pass that's
                 * part of a previous render pass, we could just pass in VK_SUBPASS_EXTERNAL here instead. Note that, this
                 * would mean "wait for all of the sub passes within all of the render passes before this one"
                */
                dependency.srcSubpass      = srcSubPassIdx;
                /* dstSubpass is the index to the current sub pass, i.e. the one this dependency exists for */
                dependency.dstSubpass      = dstSubPassIdx;
                /* srcStageMask is a bitmask of all of the pipeline "stages" we are asking Vulkan to finish executing
                 * within srcSubpass before we move on to dstSubpass. Whereas, dstStageMask is a bitmask of all of the
                 * pipeline stages in dstSubpass that we're NOT allowed to execute until after the stages in srcStageMask
                 * have completed within srcSubpass
                */
                dependency.srcStageMask    = srcStageMask;
                dependency.dstStageMask    = dstStageMask;
                /* Access masks relate to memory availability/visibility. Somewhat suprising is that just because you
                 * set up an execution dependency where for example, A (the src) writes to some resource and then B (dst)
                 * reads from the resource. Even if B executes after A, that doesn't mean B will "see" the changes A has
                 * made, because of caching! It is very possible that even though A has finished, it has made its changes
                 * to a memory cache that hasn't been made available/"flushed". So in the dependency you could use, for
                 * example
                 *
                 * srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT
                 * dstAccessMask = VK_ACCESS_MEMORY_READ_BIT
                 *
                 * The src access mask says that the memory A writes to should be made available/"flushed" to like the
                 * shared gpu memory, and the dst access mask says that the memory/cache B reads from should first pull
                 * from the shared gpu memory. This way B is reading from up to date memory, and not stale cache data
                 *
                 * Note that, VK_ACCESS_NONE means that there is no memory dependency the barrier introduces
                */
                dependency.srcAccessMask   = srcAccessMask;
                dependency.dstAccessMask   = dstAccessMask;

                m_renderPassInfo.meta.dependencies.push_back (dependency);
            }

            VkRenderPass* getRenderPass (void) {
                return &m_renderPassInfo.resource.renderPass;
            }

            void onAttach (void) override {
                createRenderPass();
            }

            void onDetach (void) override {
                destroyRenderPass();
            }

            void onUpdate (const float frameDelta) override {
                static_cast <void> (frameDelta);
                /* Do nothing */
            }

            ~VKRenderPass (void) {
                if (m_renderPassInfo.state.logObjCreated)
                    delete m_renderPassInfo.resource.logObj;
            }
    };
}   // namespace Renderer