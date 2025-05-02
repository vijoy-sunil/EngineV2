#pragma once
#include "../Common.h"

namespace Renderer {
    void setViewPorts (const VkCommandBuffer cmdBuffer,
                       const float x,
                       const float y,
                       const float width,
                       const float height,
                       const float minDepth,
                       const float maxDepth,
                       const uint32_t firstViewPortIdx,
                       std::vector <VkViewport>& viewPorts) {

        VkViewport viewPort;
        viewPort.x        = x;
        viewPort.y        = y;
        viewPort.width    = width;
        viewPort.height   = height;
        viewPort.minDepth = minDepth;
        viewPort.maxDepth = maxDepth;

        viewPorts.push_back (viewPort);
        vkCmdSetViewport    (cmdBuffer,
                             firstViewPortIdx,
                             static_cast <uint32_t> (viewPorts.size()),
                             viewPorts.data());
    }

    void setScissors (const VkCommandBuffer cmdBuffer,
                      const VkOffset2D offset,
                      const VkExtent2D extent,
                      const uint32_t firstScissorIdx,
                      std::vector <VkRect2D>& scissors) {

        VkRect2D scissor;
        scissor.offset = offset;
        scissor.extent = extent;

        scissors.push_back (scissor);
        vkCmdSetScissor    (cmdBuffer,
                            firstScissorIdx,
                            static_cast <uint32_t> (scissors.size()),
                            scissors.data());
    }

    void bindVertexBuffers (const VkCommandBuffer cmdBuffer,
                            const uint32_t firstBindingIdx,
                            const std::vector <VkBuffer>& buffers,
                            const std::vector <VkDeviceSize>& offsets) {

        vkCmdBindVertexBuffers (cmdBuffer,
                                firstBindingIdx,
                                static_cast <uint32_t> (buffers.size()),
                                buffers.data(),
                                offsets.data());
    }

    void bindIndexBuffer (const VkCommandBuffer cmdBuffer,
                          const VkBuffer buffer,
                          const VkDeviceSize offset,
                          const VkIndexType indexType) {

        vkCmdBindIndexBuffer (cmdBuffer,
                              buffer,
                              offset,
                              indexType);
    }

    void copyBufferToBuffer (const VkCommandBuffer cmdBuffer,
                             const VkBuffer srcBuffer,
                             const VkBuffer dstBuffer,
                             const VkDeviceSize srcOffset,
                             const VkDeviceSize dstOffset,
                             const VkDeviceSize size,
                             std::vector <VkBufferCopy>& copyRegions) {

        VkBufferCopy copyRegion;
        copyRegion.srcOffset = srcOffset;
        copyRegion.dstOffset = dstOffset;
        copyRegion.size      = size;

        copyRegions.push_back (copyRegion);
        vkCmdCopyBuffer       (cmdBuffer,
                               srcBuffer,
                               dstBuffer,
                               static_cast <uint32_t> (copyRegions.size()),
                               copyRegions.data());
    }

    void transitionImageLayout (const VkCommandBuffer cmdBuffer,
                                const VkImage image,
                                const uint32_t baseMipLevel,
                                const uint32_t mipLevels,
                                const uint32_t baseArrayLayer,
                                const uint32_t layersCount,
                                const VkImageLayout initialImageLayout,
                                const VkImageLayout finalImageLayout,
                                const VkImageAspectFlags aspectFlags,
                                std::vector <VkImageMemoryBarrier>& barriers) {

        VkImageMemoryBarrier barrier;
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext                           = nullptr;
        barrier.image                           = image;

        barrier.subresourceRange.baseMipLevel   = baseMipLevel;
        barrier.subresourceRange.levelCount     = mipLevels;
        barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
        barrier.subresourceRange.layerCount     = layersCount;
        barrier.subresourceRange.aspectMask     = aspectFlags;

        barrier.oldLayout                       = initialImageLayout;
        barrier.newLayout                       = finalImageLayout;
        barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;

        /* Barriers are primarily used for synchronization purposes, so we must specify which types of operations that
         * involve the resource 'must happen before the barrier', and which operations that involve the resource 'must
         * wait on the barrier'
        */
        VkPipelineStageFlags srcStage, dstStage;
        /* Transitions handled */
        if (initialImageLayout      == VK_IMAGE_LAYOUT_UNDEFINED &&
            finalImageLayout        == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {

            srcStage                 = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            barrier.srcAccessMask    = VK_ACCESS_NONE;

            dstStage                 = VK_PIPELINE_STAGE_TRANSFER_BIT;
            barrier.dstAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        else if (initialImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                 finalImageLayout   == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

            srcStage                 = VK_PIPELINE_STAGE_TRANSFER_BIT;
            barrier.srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT;

            dstStage                 = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            barrier.dstAccessMask    = VK_ACCESS_SHADER_READ_BIT;
        }
        else if (initialImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                 finalImageLayout   == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {

            srcStage                 = VK_PIPELINE_STAGE_TRANSFER_BIT;
            barrier.srcAccessMask    = VK_ACCESS_TRANSFER_WRITE_BIT;

            dstStage                 = VK_PIPELINE_STAGE_TRANSFER_BIT;
            barrier.dstAccessMask    = VK_ACCESS_TRANSFER_READ_BIT;
        }
        else if (initialImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
                 finalImageLayout   == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

            srcStage                 = VK_PIPELINE_STAGE_TRANSFER_BIT;
            barrier.srcAccessMask    = VK_ACCESS_TRANSFER_READ_BIT;

            dstStage                 = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            barrier.dstAccessMask    = VK_ACCESS_SHADER_READ_BIT;
        }
        else
            throw std::runtime_error ("Unhandled image layout transition");

        barriers.push_back   (barrier);
        vkCmdPipelineBarrier (cmdBuffer,
                              srcStage,
                              dstStage,
                              0,
                              0, nullptr,
                              0, nullptr,
                              static_cast <uint32_t> (barriers.size()),
                              barriers.data());
    }

    void copyBufferToImage (const VkCommandBuffer cmdBuffer,
                            const VkBuffer srcBuffer,
                            const VkImage dstImage,
                            const VkDeviceSize srcOffset,
                            const uint32_t bufferRowLength,
                            const uint32_t bufferImageHeight,
                            const VkOffset3D dstOffset,
                            const VkExtent3D extent,
                            const uint32_t mipLevels,
                            const uint32_t baseArrayLayer,
                            const uint32_t layersCount,
                            const VkImageLayout imageLayout,
                            const VkImageAspectFlags aspectFlags,
                            std::vector <VkBufferImageCopy>& copyRegions) {

        auto appendBarriers = std::vector <VkImageMemoryBarrier> {};
        transitionImageLayout (cmdBuffer,
                               dstImage,
                               0,
                               mipLevels,
                               baseArrayLayer,
                               layersCount,
                               VK_IMAGE_LAYOUT_UNDEFINED,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               aspectFlags,
                               appendBarriers);

        VkBufferImageCopy copyRegion;
        copyRegion.bufferOffset                    = srcOffset;
        copyRegion.bufferRowLength                 = bufferRowLength;
        copyRegion.bufferImageHeight               = bufferImageHeight;
        copyRegion.imageOffset                     = dstOffset;
        copyRegion.imageExtent                     = extent;

        copyRegion.imageSubresource.mipLevel       = 0;
        copyRegion.imageSubresource.baseArrayLayer = baseArrayLayer;
        copyRegion.imageSubresource.layerCount     = layersCount;
        copyRegion.imageSubresource.aspectMask     = aspectFlags;

        copyRegions.push_back  (copyRegion);
        vkCmdCopyBufferToImage (cmdBuffer,
                                srcBuffer,
                                dstImage,
                                imageLayout,
                                static_cast <uint32_t> (copyRegions.size()),
                                copyRegions.data());
    }

    /* Mipmaps are precalculated, downscaled versions of an image. Each new image is half the width and height of the
     * previous one. Mipmaps are used as a form of Level of Detail or LOD. Objects that are far away from the camera will
     * sample their textures from the smaller mip images. Using smaller images increases the rendering speed and avoids
     * artifacts such as moiré patterns
     *
     * Note that, if you are using a dedicated transfer queue, vkCmdBlitImage must be submitted to a queue with graphics
     * capability
    */
    void blitImageToMipMaps (const VkCommandBuffer cmdBuffer,
                             const VkImage image,
                             const uint32_t width,
                             const uint32_t height,
                             const uint32_t mipLevels,
                             const uint32_t baseArrayLayer,
                             const VkImageAspectFlags aspectFlags) {
        /* How are mip maps generated?
         * The input image is generated with multiple mip levels, but the staging buffer can only be used to fill mip
         * level 0. The other levels are still undefined. To fill these levels we need to generate the data from the
         * single level that we have. We will use the vkCmdBlitImage command to perform copying, scaling, and filtering
         * operations. We will call this multiple times to blit data to each level of our image
         *
         * Note that, vkCmdBlitImage is considered a transfer operation, so we must inform Vulkan that we intend to use
         * the image as both the source and destination of a transfer by specifying in usage flags when creating the
         * image
         *
         * vkCmdBlitImage depends on the layout of the image it operates on. We could transition the entire image to
         * VK_IMAGE_LAYOUT_GENERAL, but this will most likely be slow. For optimal performance, the source image should
         * be in VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL and the destination image should be in VK_IMAGE_LAYOUT_TRANSFER_
         * DST_OPTIMAL
         *
         * Vulkan allows us to transition each mip level of an image independently. Each blit will only deal with two
         * mip levels at a time, so we can transition each level into the optimal layout between blit commands
         *
         * > Mip level 0
         *      Transition layout [VK_IMAGE_LAYOUT_UNDEFINED]->[VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL]
         *      Copy buffer to image
         *
         * > Mip level 1 to mipLevels - 1
         *      Transition layout [VK_IMAGE_LAYOUT_UNDEFINED]->[VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL]
         *
         * > Mip level 1 to mipLevels - 1
         *      Loop {
         *
         *      Mip level: i - 1
         *      Transition layout [VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL]->[VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL]
         *
         *      Blit: mip level [i-1]->[i]
         *      Transition layout [VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL]->[VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL]
         *          .
         *          .
         *          .
         *      }
         *
         * > mipLevels - 1
         *      Transition layout [VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL]->[VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL]
        */
        int32_t mipWidth  = static_cast <int32_t> (width);
        int32_t mipHeight = static_cast <int32_t> (height);

        for (uint32_t i = 1; i < mipLevels; i++) {
            uint32_t srcMipLevel = i - 1;
            uint32_t dstMipLevel = i;
            /* First, we transition level i - 1 to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL. This transition will wait for
             * level i - 1 to be filled, either from the previous blit command, or from vkCmdCopyBufferToImage. The
             * current blit command will wait on this transition
            */
            auto appendBarriersA = std::vector <VkImageMemoryBarrier> {};
            transitionImageLayout (cmdBuffer,
                                   image,
                                   srcMipLevel,
                                   1,
                                   baseArrayLayer,
                                   1,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                   aspectFlags,
                                   appendBarriersA);

            VkImageBlit blitRegion;
            blitRegion.srcOffsets[0] = {0, 0, 0};
            blitRegion.srcOffsets[1] = {
                mipWidth,
                mipHeight,
                1
            };

            blitRegion.srcSubresource.mipLevel       = srcMipLevel;
            blitRegion.srcSubresource.baseArrayLayer = baseArrayLayer;
            blitRegion.srcSubresource.layerCount     = 1;
            blitRegion.srcSubresource.aspectMask     = aspectFlags;

            blitRegion.dstOffsets[0] = {0, 0, 0};
            blitRegion.dstOffsets[1] = {
                mipWidth  > 1 ?  mipWidth/2: 1,
                mipHeight > 1 ? mipHeight/2: 1,
                1
            };

            blitRegion.dstSubresource.mipLevel       = dstMipLevel;
            blitRegion.dstSubresource.baseArrayLayer = baseArrayLayer;
            blitRegion.dstSubresource.layerCount     = 1;
            blitRegion.dstSubresource.aspectMask     = aspectFlags;

            vkCmdBlitImage        (cmdBuffer,
                                   image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                   image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   1,     &blitRegion,
                                   VK_FILTER_LINEAR);

            /* To be able to start sampling from the texture in the shader, we need one last transition to prepare it for
             * shader access. Note that, this transition waits on the current blit command to finish
            */
            auto appendBarriersB = std::vector <VkImageMemoryBarrier> {};
            transitionImageLayout (cmdBuffer,
                                   image,
                                   srcMipLevel,
                                   1,
                                   baseArrayLayer,
                                   1,
                                   VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                   VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                   aspectFlags,
                                   appendBarriersB);

            /* At the end of the loop, we divide the current mip dimensions by two. We check each dimension before the
             * division to ensure that dimension never becomes 0. This handles cases where the image is not square, since
             * one of the mip dimensions would reach 1 before the other dimension. When this happens, that dimension
             * should remain 1 for all remaining levels
            */
            if (mipWidth > 1)  mipWidth  /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }
        /* Transition the last mip level, since the last level was never blitted from */
        auto appendBarriers = std::vector <VkImageMemoryBarrier> {};
        transitionImageLayout (cmdBuffer,
                               image,
                               mipLevels - 1,
                               1,
                               baseArrayLayer,
                               1,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                               aspectFlags,
                               appendBarriers);
    }

    void beginRenderPass (const VkCommandBuffer cmdBuffer,
                          const VkRenderPass renderPass,
                          const VkFramebuffer frameBuffer,
                          const VkOffset2D offset,
                          const VkExtent2D extent,
                          const std::vector <VkClearValue>& clearValues) {

        VkRenderPassBeginInfo beginInfo;
        beginInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.pNext             = nullptr;
        beginInfo.renderPass        = renderPass;
        beginInfo.framebuffer       = frameBuffer;
        beginInfo.renderArea.offset = offset;
        beginInfo.renderArea.extent = extent;
        beginInfo.clearValueCount   = static_cast <uint32_t> (clearValues.size());
        beginInfo.pClearValues      = clearValues.data();

        vkCmdBeginRenderPass (cmdBuffer,
                              &beginInfo,
                              VK_SUBPASS_CONTENTS_INLINE);
    }

    void endRenderPass (const VkCommandBuffer cmdBuffer) {
        vkCmdEndRenderPass (cmdBuffer);
    }

    void updatePushConstants (const VkCommandBuffer cmdBuffer,
                              const VkPipelineLayout pipelineLayout,
                              const VkShaderStageFlags shaderStages,
                              const uint32_t offset,
                              const uint32_t size,
                              const void* data) {

        vkCmdPushConstants (cmdBuffer,
                            pipelineLayout,
                            shaderStages,
                            offset, size, data);
    }

    void bindPipeline (const VkCommandBuffer cmdBuffer,
                       const VkPipelineBindPoint pipelineBindPoint,
                       const VkPipeline pipeline) {

        vkCmdBindPipeline (cmdBuffer,
                           pipelineBindPoint,
                           pipeline);
    }

    void bindDescriptorSets (const VkCommandBuffer cmdBuffer,
                             const VkPipelineBindPoint pipelineBindPoint,
                             const VkPipelineLayout pipelineLayout,
                             const uint32_t firstSetIdx,
                             const std::vector <VkDescriptorSet>& descriptorSets,
                             const std::vector <uint32_t>& dynamicOffsets) {

        vkCmdBindDescriptorSets (cmdBuffer,
                                 pipelineBindPoint,
                                 pipelineLayout,
                                 firstSetIdx,
                                 static_cast <uint32_t> (descriptorSets.size()),
                                 descriptorSets.data(),
                                 static_cast <uint32_t> (dynamicOffsets.size()),
                                 dynamicOffsets.data());
    }

    void drawSimple (const VkCommandBuffer cmdBuffer,
                     const uint32_t firstVertexIdx,
                     const uint32_t verticesCount,
                     const uint32_t firstInstanceIdx,
                     const uint32_t instancesCount) {

        vkCmdDraw (cmdBuffer,
                   verticesCount,
                   instancesCount,
                   firstVertexIdx,
                   firstInstanceIdx);
    }

    void drawIndexed (const VkCommandBuffer cmdBuffer,
                      const uint32_t firstIndexIdx,
                      const uint32_t indicesCount,
                      const int32_t  vertexOffset,
                      const uint32_t firstInstanceIdx,
                      const uint32_t instancesCount) {

        vkCmdDrawIndexed (cmdBuffer,
                          indicesCount,
                          instancesCount,
                          firstIndexIdx,
                          vertexOffset,
                          firstInstanceIdx);
    }
}   // namespace Renderer