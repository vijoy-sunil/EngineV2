#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include "../Collection/CNTypeInstanceBase.h"
#include "../Log/LGImpl.h"
#include "VKLogDevice.h"

namespace Renderer {
    class VKPipeline: public Collection::CNTypeInstanceBase {
        private:
            struct PipelineInfo {
                struct Meta {
                    VkPipelineCreateFlags createFlags;
                    uint32_t subPassIdx;
                    VkRenderPass renderPass;
                    /* Vulkan allows you to create a new graphics pipeline by deriving from an existing pipeline. The
                     * idea of pipeline derivatives is that it is less expensive to set up pipelines when they have much
                     * functionality in common with an existing pipeline and switching between pipelines from the same
                     * parent can also be done quicker
                    */
                    int32_t basePipelineIdx;
                    VkPipeline basePipeline;

                    VkPipelineVertexInputStateCreateInfo          vertexInput;
                    VkPipelineInputAssemblyStateCreateInfo        inputAssembly;
                    std::vector <VkPipelineShaderStageCreateInfo> shaderStages;
                    VkPipelineDepthStencilStateCreateInfo         depthStencil;
                    VkPipelineRasterizationStateCreateInfo        rasterization;
                    VkPipelineMultisampleStateCreateInfo          multiSample;
                    VkPipelineColorBlendStateCreateInfo           colorBlend;
                    VkPipelineDynamicStateCreateInfo              dynamicState;
                    VkPipelineViewportStateCreateInfo             viewPort;
                    std::vector <VkPushConstantRange>             pushConstantRanges;
                } meta;

                struct State {
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKLogDevice* logDeviceObj;
                    VkPipeline pipeline;
                    VkPipelineLayout layout;
                    std::vector <VkShaderModule>        shaderModules;
                    std::vector <VkDescriptorSetLayout> descriptorSetLayouts;
                } resource;
            } m_pipelineInfo;

            VkShaderModule createShaderModule (const char* binaryFilePath) {
                /* The advantage of starting to read at the end of the file is that we can use the read position to
                 * determine the size of the file and allocate a buffer
                */
                std::ifstream file (binaryFilePath, std::ios::ate | std::ios::binary);
                if (!file.is_open()) {
                    LOG_WARNING (m_pipelineInfo.resource.logObj) << "Failed to open file"
                                                                 << " "
                                                                 << "[" << binaryFilePath << "]"
                                                                 << std::endl;
                }
                size_t fileSize = static_cast <size_t> (file.tellg());
                std::vector <char> shaderByteCode (fileSize);

                file.seekg (0);
                file.read (shaderByteCode.data(), fileSize);
                file.close();

                VkShaderModuleCreateInfo createInfo;
                createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                createInfo.pNext    = nullptr;
                createInfo.flags    = 0;
                createInfo.codeSize = shaderByteCode.size();
                createInfo.pCode    = reinterpret_cast <const uint32_t*> (shaderByteCode.data());

                VkShaderModule shaderModule;
                auto result = vkCreateShaderModule (*m_pipelineInfo.resource.logDeviceObj->getLogDevice(),
                                                     &createInfo,
                                                     nullptr,
                                                     &shaderModule);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_pipelineInfo.resource.logObj) << "[?] Shader module"
                                                               << " "
                                                               << "[" << string_VkResult (result) << "]"
                                                               << std::endl;
                    throw std::runtime_error ("[?] Shader module");
                }
                LOG_INFO (m_pipelineInfo.resource.logObj)      << "[O] Shader module"
                                                               << std::endl;
                return shaderModule;
            }

            void createPipelineLayout (void) {
                VkPipelineLayoutCreateInfo createInfo;
                createInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                createInfo.pNext                  = nullptr;
                createInfo.flags                  = 0;
                createInfo.setLayoutCount         = static_cast <uint32_t>
                                                    (m_pipelineInfo.resource.descriptorSetLayouts.size());
                createInfo.pSetLayouts            = m_pipelineInfo.resource.descriptorSetLayouts.data();
                createInfo.pushConstantRangeCount = static_cast <uint32_t>
                                                    (m_pipelineInfo.meta.pushConstantRanges.size());
                createInfo.pPushConstantRanges    = m_pipelineInfo.meta.pushConstantRanges.data();

                auto result =  vkCreatePipelineLayout (*m_pipelineInfo.resource.logDeviceObj->getLogDevice(),
                                                        &createInfo,
                                                        nullptr,
                                                        &m_pipelineInfo.resource.layout);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_pipelineInfo.resource.logObj) << "[?] Pipeline layout"
                                                               << " "
                                                               << "[" << string_VkResult (result) << "]"
                                                               << std::endl;
                    throw std::runtime_error ("[?] Pipeline layout");
                }
                LOG_INFO (m_pipelineInfo.resource.logObj)      << "[O] Pipeline layout"
                                                               << std::endl;
            }

            void createPipeline (void) {
                createPipelineLayout();
                VkGraphicsPipelineCreateInfo createInfo;
                createInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                createInfo.pNext               = nullptr;
                createInfo.flags               = m_pipelineInfo.meta.createFlags;
                createInfo.subpass             = m_pipelineInfo.meta.subPassIdx;
                createInfo.renderPass          = m_pipelineInfo.meta.renderPass;
                createInfo.basePipelineIndex   = m_pipelineInfo.meta.basePipelineIdx;
                createInfo.basePipelineHandle  = m_pipelineInfo.meta.basePipeline;

                createInfo.pVertexInputState   = &m_pipelineInfo.meta.vertexInput;
                createInfo.pInputAssemblyState = &m_pipelineInfo.meta.inputAssembly;
                createInfo.pTessellationState  = nullptr;
                createInfo.stageCount          = static_cast <uint32_t> (m_pipelineInfo.meta.shaderStages.size());
                createInfo.pStages             = m_pipelineInfo.meta.shaderStages.data();
                createInfo.pDepthStencilState  = &m_pipelineInfo.meta.depthStencil;
                createInfo.pRasterizationState = &m_pipelineInfo.meta.rasterization;
                createInfo.pMultisampleState   = &m_pipelineInfo.meta.multiSample;
                createInfo.pColorBlendState    = &m_pipelineInfo.meta.colorBlend;
                createInfo.pDynamicState       = &m_pipelineInfo.meta.dynamicState;
                createInfo.pViewportState      = &m_pipelineInfo.meta.viewPort;
                createInfo.layout              = m_pipelineInfo.resource.layout;

                auto result = vkCreateGraphicsPipelines (*m_pipelineInfo.resource.logDeviceObj->getLogDevice(),
                                                          nullptr,
                                                          1,
                                                          &createInfo,
                                                          nullptr,
                                                          &m_pipelineInfo.resource.pipeline);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_pipelineInfo.resource.logObj) << "[?] Pipeline"
                                                               << " "
                                                               << "[" << string_VkResult (result) << "]"
                                                               << std::endl;
                    throw std::runtime_error ("[?] Pipeline");
                }
                LOG_INFO (m_pipelineInfo.resource.logObj)      << "[O] Pipeline"
                                                               << std::endl;
            }

            void destroyPipeline (void) {
                for (auto const& descriptorSetLayout: m_pipelineInfo.resource.descriptorSetLayouts) {
                    vkDestroyDescriptorSetLayout (*m_pipelineInfo.resource.logDeviceObj->getLogDevice(),
                                                   descriptorSetLayout,
                                                   nullptr);
                    LOG_INFO (m_pipelineInfo.resource.logObj)  << "[X] Descriptor set layout"
                                                               << std::endl;
                }

                vkDestroyPipelineLayout (*m_pipelineInfo.resource.logDeviceObj->getLogDevice(),
                                          m_pipelineInfo.resource.layout,
                                          nullptr);
                LOG_INFO (m_pipelineInfo.resource.logObj)      << "[X] Pipeline layout"
                                                               << std::endl;

                vkDestroyPipeline       (*m_pipelineInfo.resource.logDeviceObj->getLogDevice(),
                                          m_pipelineInfo.resource.pipeline,
                                          nullptr);
                LOG_INFO (m_pipelineInfo.resource.logObj)      << "[X] Pipeline"
                                                               << std::endl;
            }

        public:
            VKPipeline (Log::LGImpl* logObj,
                        VKLogDevice* logDeviceObj) {

                m_pipelineInfo = {};

                if (logObj == nullptr) {
                    m_pipelineInfo.resource.logObj     = new Log::LGImpl();
                    m_pipelineInfo.state.logObjCreated = true;

                    m_pipelineInfo.resource.logObj->initLogInfo ("Build/Log/Renderer", __FILE__);
                    LOG_WARNING (m_pipelineInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                                 << std::endl;
                }
                else {
                    m_pipelineInfo.resource.logObj     = logObj;
                    m_pipelineInfo.state.logObjCreated = false;
                }

                if (logDeviceObj == nullptr) {
                    LOG_ERROR (m_pipelineInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                               << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_pipelineInfo.resource.logDeviceObj   = logDeviceObj;
            }

            void initPipelineInfo (const VkPipelineCreateFlags createFlags,
                                   const uint32_t subPassIdx,
                                   const VkRenderPass renderPass,
                                   const int32_t basePipelineIdx,
                                   const VkPipeline basePipeline) {

                m_pipelineInfo.meta.createFlags              = createFlags;
                m_pipelineInfo.meta.subPassIdx               = subPassIdx;
                m_pipelineInfo.meta.renderPass               = renderPass;
                m_pipelineInfo.meta.basePipelineIdx          = basePipelineIdx;
                m_pipelineInfo.meta.basePipeline             = basePipeline;

                m_pipelineInfo.meta.vertexInput              = {};
                m_pipelineInfo.meta.inputAssembly            = {};
                m_pipelineInfo.meta.shaderStages             = {};
                m_pipelineInfo.meta.depthStencil             = {};
                m_pipelineInfo.meta.rasterization            = {};
                m_pipelineInfo.meta.multiSample              = {};
                m_pipelineInfo.meta.colorBlend               = {};
                m_pipelineInfo.meta.dynamicState             = {};
                m_pipelineInfo.meta.viewPort                 = {};
                m_pipelineInfo.meta.pushConstantRanges       = {};

                m_pipelineInfo.resource.pipeline             = nullptr;
                m_pipelineInfo.resource.layout               = nullptr;
                m_pipelineInfo.resource.shaderModules        = {};
                m_pipelineInfo.resource.descriptorSetLayouts = {};
            }

            /* Binding describes the spacing between data and whether the data is per-vertex or per-instance */
            VkVertexInputBindingDescription createVertexBindingDescription (const uint32_t bindingNumber,
                                                                            const uint32_t stride,
                                                                            const VkVertexInputRate inputRate) {
                VkVertexInputBindingDescription bindingDescription;
                bindingDescription.binding   = bindingNumber;
                bindingDescription.stride    = stride;
                bindingDescription.inputRate = inputRate;

                return bindingDescription;
            }

            /* Attribute describes the type of the attribute passed to the vertex shader, which binding to load them
             * from and at which offset
            */
            VkVertexInputAttributeDescription createVertexAttributeDescription (const uint32_t bindingNumber,
                                                                                const uint32_t location,
                                                                                const uint32_t offset,
                                                                                const VkFormat format) {
                VkVertexInputAttributeDescription attributeDescription;
                attributeDescription.binding  = bindingNumber;
                attributeDescription.location = location;
                attributeDescription.offset   = offset;
                attributeDescription.format   = format;

                return attributeDescription;
            }

            void createVertexInputState (const std::vector <VkVertexInputBindingDescription>&   bindingDescriptions,
                                         const std::vector <VkVertexInputAttributeDescription>& attributeDescriptions) {

                VkPipelineVertexInputStateCreateInfo createInfo;
                createInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                createInfo.pNext                           = nullptr;
                createInfo.flags                           = 0;
                createInfo.vertexBindingDescriptionCount   = static_cast <uint32_t> (bindingDescriptions.size());
                createInfo.pVertexBindingDescriptions      = bindingDescriptions.data();
                createInfo.vertexAttributeDescriptionCount = static_cast <uint32_t> (attributeDescriptions.size());
                createInfo.pVertexAttributeDescriptions    = attributeDescriptions.data();

                m_pipelineInfo.meta.vertexInput            = createInfo;
            }

            void createInputAssemblyState (const VkPrimitiveTopology topology,
                                           const VkBool32 restartEnable) {

                VkPipelineInputAssemblyStateCreateInfo createInfo;
                createInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                createInfo.pNext                  = nullptr;
                createInfo.flags                  = 0;
                createInfo.topology               = topology;
                createInfo.primitiveRestartEnable = restartEnable;

                m_pipelineInfo.meta.inputAssembly = createInfo;
            }

            void addShaderStage (const VkShaderStageFlagBits shaderStage,
                                 const char* binaryFilePath,
                                 const char* entryPoint) {

                auto module                    = createShaderModule (binaryFilePath);
                /* Save modules to destroy later */
                m_pipelineInfo.resource.shaderModules.push_back (module);

                VkPipelineShaderStageCreateInfo createInfo;
                createInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                createInfo.pNext               = nullptr;
                createInfo.flags               = 0;
                createInfo.stage               = shaderStage;
                createInfo.module              = module;
                /* The shader function to invoke (aka entry point) is specified here. That means that it's possible to
                 * combine multiple shaders into a single shader module and use different entry points to differentiate
                 * between their behaviors
                */
                createInfo.pName               = entryPoint;
                /* This field allows you to specify values for shader constants. You can use a single shader module where
                 * its behavior can be configured at pipeline creation by specifying different values for the constants
                 * used in it. This is more efficient than configuring the shader using variables at render time, because
                 * the compiler can do optimizations like eliminating if statements that depend on these values
                */
                createInfo.pSpecializationInfo = nullptr;

                m_pipelineInfo.meta.shaderStages.push_back (createInfo);
            }

            void createDepthStencilState (const VkBool32 depthTestEnable,
                                          const VkBool32 depthWriteEnable,
                                          const VkBool32 depthBoundsTestEnable,
                                          const VkBool32 stencilTestEnable,
                                          const VkCompareOp depthCompareOp,
                                          const float minDepthBounds,
                                          const float maxDepthBounds,
                                          const VkStencilOpState stencilOpFront,
                                          const VkStencilOpState stencilOpBack) {

                VkPipelineDepthStencilStateCreateInfo createInfo;
                createInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                createInfo.pNext                 = nullptr;
                createInfo.flags                 = 0;
                createInfo.depthTestEnable       = depthTestEnable;
                createInfo.depthWriteEnable      = depthWriteEnable;
                createInfo.depthBoundsTestEnable = depthBoundsTestEnable;
                createInfo.stencilTestEnable     = stencilTestEnable;
                createInfo.depthCompareOp        = depthCompareOp;
                createInfo.minDepthBounds        = minDepthBounds;
                createInfo.maxDepthBounds        = maxDepthBounds;
                createInfo.front                 = stencilOpFront;
                createInfo.back                  = stencilOpBack;

                m_pipelineInfo.meta.depthStencil = createInfo;
            }

            void createRasterizationState (const float lineWidth,
                                           const VkPolygonMode polygonMode,
                                           const VkCullModeFlags cullMode,
                                           const VkFrontFace frontFace) {

                VkPipelineRasterizationStateCreateInfo createInfo;
                createInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                createInfo.pNext                   = nullptr;
                createInfo.flags                   = 0;
                createInfo.depthClampEnable        = VK_FALSE;
                createInfo.rasterizerDiscardEnable = VK_FALSE;
                createInfo.lineWidth               = lineWidth;
                createInfo.polygonMode             = polygonMode;
                createInfo.cullMode                = cullMode;
                createInfo.frontFace               = frontFace;
                createInfo.depthBiasEnable         = VK_FALSE;
                createInfo.depthBiasConstantFactor = 0.0f;
                createInfo.depthBiasClamp          = 0.0f;
                createInfo.depthBiasSlopeFactor    = 0.0f;

                m_pipelineInfo.meta.rasterization  = createInfo;
            }

            void createMultiSampleState (const VkSampleCountFlagBits samplesCount,
                                         const VkBool32 sampleShadingEnable,
                                         const float minSampleShading ) {

                VkPipelineMultisampleStateCreateInfo createInfo;
                createInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                createInfo.pNext                 = nullptr;
                createInfo.flags                 = 0;
                createInfo.rasterizationSamples  = samplesCount;
                createInfo.sampleShadingEnable   = sampleShadingEnable;
                /* MSAA only smoothens out the edges of geometry but not the interior filling. This may lead to a
                 * situation when you get a smooth polygon rendered on screen but the applied texture will still look
                 * aliased if it contains high contrasting colors. One way to approach this problem is to enable sample
                 * shading which will improve the image quality even further, though at an additional performance cost
                */
                createInfo.minSampleShading      = minSampleShading;
                createInfo.pSampleMask           = nullptr;
                createInfo.alphaToCoverageEnable = VK_FALSE;
                createInfo.alphaToOneEnable      = VK_FALSE;

                m_pipelineInfo.meta.multiSample  = createInfo;
            }

            /* After a fragment shader has returned a color, it needs to be combined with the color that is already in
             * the frame buffer. This transformation is known as color blending and there are two ways to do it:
             *
             * (1) Mix the old and new value to produce a final color (using VkPipelineColorBlendAttachmentState)
             * (2) Combine the old and new value using a bitwise operation (using VkPipelineColorBlendStateCreateInfo)
            */
            VkPipelineColorBlendAttachmentState createColorBlendAttachment (const VkBool32 blendEnable,
                                                                            const VkBlendFactor srcColorBlendFactor,
                                                                            const VkBlendFactor dstColorBlendFactor,
                                                                            const VkBlendOp colorBlendOp,
                                                                            const VkBlendFactor srcAlphaBlendFactor,
                                                                            const VkBlendFactor dstAlphaBlendFactor,
                                                                            const VkBlendOp alphaBlendOp,
                                                                            const VkColorComponentFlags colorWriteMask) {
                VkPipelineColorBlendAttachmentState attachment;
                /* If blendEnable is VK_FALSE, then the new color from the fragment shader is passed through unmodified */
                attachment.blendEnable         = blendEnable;
                /* finalColor.rgb              = (srcColorBlendFactor * newColor.rgb) <colorBlendOp>
                 *                               (dstColorBlendFactor * oldColor.rgb)
                 * finalColor.a                = (srcAlphaBlendFactor * newColor.a)   <alphaBlendOp>
                 *                               (dstAlphaBlendFactor * oldColor.a)
                 * finalColor                  = finalColor & colorWriteMask
                */
                attachment.srcColorBlendFactor = srcColorBlendFactor;
                attachment.dstColorBlendFactor = dstColorBlendFactor;
                attachment.colorBlendOp        = colorBlendOp;
                attachment.srcAlphaBlendFactor = srcAlphaBlendFactor;
                attachment.dstAlphaBlendFactor = dstAlphaBlendFactor;
                attachment.alphaBlendOp        = alphaBlendOp;
                attachment.colorWriteMask      = colorWriteMask;

                return attachment;
            }

            void createColorBlendState (const VkBool32 logicOpEnable,
                                        const VkLogicOp logicOp,
                                        const std::vector <VkPipelineColorBlendAttachmentState>& attachments,
                                        const std::vector <float> blendConstants) {

                VkPipelineColorBlendStateCreateInfo createInfo;
                createInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                createInfo.pNext               = nullptr;
                createInfo.flags               = 0;
                /* Note that setting logicOpEnable to VK_TRUE will automatically disable the first method of blending, as
                 * if you had set blendEnable to VK_FALSE for every attached frame buffer. However, the colorWriteMask
                 * will be used to determine which channels in the frame buffer will actually be affected
                */
                createInfo.logicOpEnable       = logicOpEnable;
                createInfo.logicOp             = logicOp;
                createInfo.attachmentCount     = static_cast <uint32_t> (attachments.size());
                createInfo.pAttachments        = attachments.data();

                createInfo.blendConstants[0]   = blendConstants[0];
                createInfo.blendConstants[1]   = blendConstants[1];
                createInfo.blendConstants[2]   = blendConstants[2];
                createInfo.blendConstants[3]   = blendConstants[3];

                m_pipelineInfo.meta.colorBlend = createInfo;
            }

            /* The graphics pipeline in Vulkan is almost completely immutable, so you must recreate the pipeline from
             * scratch if, for example, you want to change shaders, bind different frame buffers or change the blend
             * function. The disadvantage is that you'll have to create a number of pipelines that represent all of the
             * different combinations of states you want to use in your rendering operations. However, because all of
             * the operations you'll be doing in the pipeline are known in advance, the driver can optimize for it much
             * better
             *
             * However, a limited amount of the state can actually be changed without recreating the pipeline at draw
             * time. This will cause the configuration of these values to be ignored and you will be able (and required)
             * to specify the data at drawing time. This results in a more flexible setup and is very common for things
             * like viewport and scissor state
            */
            void createDynamicState (const std::vector <VkDynamicState>& dynamicStates) {
                VkPipelineDynamicStateCreateInfo createInfo;
                createInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                createInfo.pNext                 = nullptr;
                createInfo.flags                 = 0;
                createInfo.dynamicStateCount     = static_cast <uint32_t> (dynamicStates.size());
                createInfo.pDynamicStates        = dynamicStates.data();

                m_pipelineInfo.meta.dynamicState = createInfo;
            }

            void createViewPortState (const std::vector <VkViewport>& viewPorts,
                                      const std::vector <VkRect2D>& scissors,
                                      const uint32_t viewPortsCount,
                                      const uint32_t scissorsCount) {

                VkPipelineViewportStateCreateInfo createInfo;
                createInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                createInfo.pNext             = nullptr;
                createInfo.flags             = 0;
                /* Note that, if we are using dynamic state viewport and scissor we would be passing in empty vectors,
                 * however the viewport and scissor count shouldn't be zero. Hence why we are manually passing in the
                 * count values instead of computing the respective vector sizes
                */
                createInfo.viewportCount     = viewPortsCount;
                createInfo.pViewports        = viewPorts.data();
                createInfo.scissorCount      = scissorsCount;
                createInfo.pScissors         = scissors.data();

                m_pipelineInfo.meta.viewPort = createInfo;
            }

            VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding (const uint32_t bindingNumber,
                                                                           const uint32_t descriptorsCount,
                                                                           const VkDescriptorType descriptorType,
                                                                           const VkShaderStageFlags shaderStages) {
                VkDescriptorSetLayoutBinding layoutBinding;
                layoutBinding.binding            = bindingNumber;
                layoutBinding.descriptorCount    = descriptorsCount;
                layoutBinding.descriptorType     = descriptorType;
                layoutBinding.stageFlags         = shaderStages;
                layoutBinding.pImmutableSamplers = nullptr;

                return layoutBinding;
            }

            void addDescriptorSetLayout (const VkDescriptorSetLayoutCreateFlags createFlags,
                                         const std::vector <VkDescriptorBindingFlags>&     bindingFlags,
                                         const std::vector <VkDescriptorSetLayoutBinding>& layoutBindings) {

                VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo;
                bindingFlagsCreateInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                bindingFlagsCreateInfo.pNext         = nullptr;
                bindingFlagsCreateInfo.bindingCount  = static_cast <uint32_t> (bindingFlags.size());
                bindingFlagsCreateInfo.pBindingFlags = bindingFlags.data();

                VkDescriptorSetLayoutCreateInfo createInfo;
                createInfo.sType                     = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                createInfo.pNext                     = &bindingFlagsCreateInfo;
                createInfo.flags                     = createFlags;
                createInfo.bindingCount              = static_cast <uint32_t> (layoutBindings.size());
                createInfo.pBindings                 = layoutBindings.data();

                VkDescriptorSetLayout descriptorSetLayout;
                auto result = vkCreateDescriptorSetLayout (*m_pipelineInfo.resource.logDeviceObj->getLogDevice(),
                                                            &createInfo,
                                                            nullptr,
                                                            &descriptorSetLayout);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_pipelineInfo.resource.logObj) << "[?] Descriptor set layout"
                                                               << " "
                                                               << "[" << string_VkResult (result) << "]"
                                                               << std::endl;
                    throw std::runtime_error ("[?] Descriptor set layout");
                }
                LOG_INFO (m_pipelineInfo.resource.logObj)      << "[O] Descriptor set layout"
                                                               << std::endl;

                m_pipelineInfo.resource.descriptorSetLayouts.push_back (descriptorSetLayout);
            }

            /* Shaders in Vulkan usually access information stored in memory through a descriptor resource. Push constants
             * aren’t descriptors though; they live outside of that system. Instead of having a piece of user-allocated
             * memory storage, push constant storage is ephemeral. When you bind a program pipeline, you are effectively
             * creating a few bytes of push constant storage memory. You can upload CPU data to this memory via
             * vkCmdPushConstants. Rendering or dispatch commands issued after this function can read from this memory
             * through push constant uniform values. No synchronization is needed, as vkCmdPushConstants effectively
             * executes immediately (within the command buffer)
             *
             * Note that, push constants are written in ranges. An important reason for that, is that you can have
             * different push constants, at different ranges, in different stages. For example, you can reserve 64 bytes
             * (1 glm::mat4) size on the vertex shader, and then start the frag shader push constant from offset 64. This
             * way you would have different push constants on different stages
            */
            void addPushConstantRange (const VkShaderStageFlags shaderStages,
                                       const uint32_t offset,
                                       const uint32_t size) {

                VkPushConstantRange range;
                range.stageFlags = shaderStages;
                range.offset     = offset;
                range.size       = size;

                m_pipelineInfo.meta.pushConstantRanges.push_back (range);
            }

            std::vector <VkDescriptorSetLayout>& getDescriptorSetLayouts (void) {
                return m_pipelineInfo.resource.descriptorSetLayouts;
            }

            VkPipelineLayout* getPipelineLayout (void) {
                return &m_pipelineInfo.resource.layout;
            }

            VkPipeline* getPipeline (void) {
                return &m_pipelineInfo.resource.pipeline;
            }

            void destroyShaderModules (void) {
                for (auto const& shaderModule: m_pipelineInfo.resource.shaderModules) {
                    vkDestroyShaderModule (*m_pipelineInfo.resource.logDeviceObj->getLogDevice(),
                                            shaderModule,
                                            nullptr);
                    LOG_INFO (m_pipelineInfo.resource.logObj) << "[X] Shader module"
                                                              << std::endl;
                }
            }

            void onAttach (void) override {
                createPipeline();
            }

            void onDetach (void) override {
                destroyPipeline();
            }

            void onUpdate (const float frameDelta) override {
                static_cast <void> (frameDelta);
                /* Do nothing */
            }

            ~VKPipeline (void) {
                if (m_pipelineInfo.state.logObjCreated)
                    delete m_pipelineInfo.resource.logObj;
            }
    };
}   // namespace Renderer