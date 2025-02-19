#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <set>
#include <vector>

namespace Renderer {
    bool isIndicesUnique (const std::vector <uint32_t> indices) {
        std::set <uint32_t> setContainer (indices.begin(), indices.end());
        /* If the input indices are all the same, then the set container would only contain 1-item */
        return setContainer.size() != 1;
    }

    /* Note that, we're using a depth buffer, so we have to take into account the sample count for both color and depth.
     * The highest sample count that is supported by both (&) will be the maximum we can support
    */
    VkSampleCountFlagBits getMaxSupportedSamplesCount (const VkPhysicalDevice phyDevice) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties (phyDevice, &properties);

        auto samplesCount = properties.limits.framebufferColorSampleCounts &
                            properties.limits.framebufferDepthSampleCounts;

        if (samplesCount & VK_SAMPLE_COUNT_64_BIT)  return VK_SAMPLE_COUNT_64_BIT;
        if (samplesCount & VK_SAMPLE_COUNT_32_BIT)  return VK_SAMPLE_COUNT_32_BIT;
        if (samplesCount & VK_SAMPLE_COUNT_16_BIT)  return VK_SAMPLE_COUNT_16_BIT;
        if (samplesCount & VK_SAMPLE_COUNT_8_BIT)   return VK_SAMPLE_COUNT_8_BIT;
        if (samplesCount & VK_SAMPLE_COUNT_4_BIT)   return VK_SAMPLE_COUNT_4_BIT;
        if (samplesCount & VK_SAMPLE_COUNT_2_BIT)   return VK_SAMPLE_COUNT_2_BIT;

        return VK_SAMPLE_COUNT_1_BIT;
    }

    float getMaxSamplerAnisotropy (const VkPhysicalDevice phyDevice) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties (phyDevice, &properties);

        return properties.limits.maxSamplerAnisotropy;
    }

    uint32_t getMemoryTypeIdx (const VkPhysicalDevice phyDevice,
                               const uint32_t memoryTypeFilter,
                               const VkMemoryPropertyFlags memoryProperties) {

        VkPhysicalDeviceMemoryProperties properties;
        vkGetPhysicalDeviceMemoryProperties (phyDevice, &properties);

        /* Vulkan recognizes two distinct concepts when it comes to memory
         * (1) There are the actual physical pieces of RAM that the device can talk to
         * (2) Then there are ways to allocate memory from one of those pools of RAM
         *
         * A heap represents a specific piece of RAM. VkMemoryHeap is the object that describes one of the available
         * heaps of RAM that the device can talk to. There really aren't that many things that define a particular heap.
         * Just two: the number of bytes of that RAMs storage and the storage's location relative to the Vulkan device
         * (local vs. non-local)
         *
         * A memory type is a particular means of allocating memory from a specific heap. VkMemoryType is the object that
         * describes a particular way of allocating memory. And there are a lot more descriptive flags for how you can
         * allocate memory from a heap
         *
         * Example
         * Consider a standard PC setup with a discrete GPU. The device has its own local RAM, but the discrete GPU can
         * also access CPU memory. So a Vulkan device will have two heaps: one of them will be local, the other non-local
         *
         * However, there will usually be more than two memory types. You usually have one memory type that represents
         * local memory, which does not have the VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT set. That means you can't map the
         * memory; you can only access it via transfer operations from some other memory type
         *
         * But you will often have two memory types that both use the same non-local heap. They will both be
         * VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, thus allowing mapping. However, one of them will likely have the
         * VK_MEMORY_PROPERTY_HOST_CACHED_BIT flag set, while the other will be VK_MEMORY_PROPERTY_HOST_COHERENT_BIT.
         * This allows you to choose whether you want cached CPU access (thus requiring an explicit flush of ranges of
         * modified memory) or uncached CPU access
         *
         * But while they are two separate memory types, they both allocate from the same heap. Which is why VkMemoryType
         * has an index that refers to the heap who's memory it is allocating from. Note that, currently, we are only
         * concerning ourselves with the type of memory and not the heap it comes from, but you can imagine that this can
         * affect performance
         *
         * PROPERTY_DEVICE_LOCAL vs MEMORY_DEVICE_LOCAL
         * PROPERTY_DEVICE_LOCAL denotes a memory type which will achieve the best device access performance. The only
         * connection between this and MEMORY_DEVICE_LOCAL is that memory types with PROPERTY_DEVICE_LOCAL will only be
         * associated with memory heaps that use MEMORY_DEVICE_LOCAL
         *
         * An example of when a memory heap would be device local, yet have memory types that aren't, consider a GPU
         * that has no memory of its own. There's only one heap, which is therefore MEMORY_DEVICE_LOCAL. However,
         * allocating memory from that pool in a way that makes it host-visible may decrease the performance of device
         * access to that memory. Therefore, for such hardware, the host-visible memory types for the same heap will not
         * use PROPERTY_DEVICE_LOCAL. Then again, other hardware doesn't lose performance from making memory host-visible.
         * So they only have one memory type, which has all of the available properties
        */
        for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {
            if ((memoryTypeFilter & (1 << i)) &&
                (properties.memoryTypes[i].propertyFlags & memoryProperties) == memoryProperties) {
                return i;
            }
        }
        throw std::runtime_error ("Failed to find memory type idx");
    }

    /* Take a list of candidate formats in order from most desirable to least desirable, and return the first one that
     * supports desired format features and tiling mode
    */
    VkFormat getSupportedFormat (const VkPhysicalDevice phyDevice,
                                 const std::vector <VkFormat> formatCandidates,
                                 const VkFormatFeatureFlags formatFeatures,
                                 const VkImageTiling tiling) {

        for (auto const& format: formatCandidates) {
            VkFormatProperties properties;
            vkGetPhysicalDeviceFormatProperties (phyDevice,
                                                 format,
                                                 &properties);

            if (tiling == VK_IMAGE_TILING_LINEAR &&
                (properties.linearTilingFeatures & formatFeatures)  == formatFeatures)
                return format;

            if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                (properties.optimalTilingFeatures & formatFeatures) == formatFeatures)
                return format;
        }
        throw std::runtime_error ("Failed to find supported format");
    }

    void resetCmdBufferRecording (const VkCommandBuffer cmdBuffer,
                                  const VkCommandBufferResetFlags flags) {

        auto result = vkResetCommandBuffer (cmdBuffer, flags);
        if (result != VK_SUCCESS)
            throw std::runtime_error ("Failed to reset cmd buffer recording");
    }

    void beginCmdBufferRecording (const VkCommandBuffer cmdBuffer,
                                  const VkCommandBufferUsageFlags bufferUsages) {

        VkCommandBufferBeginInfo beginInfo;
        beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.pNext            = nullptr;
        beginInfo.flags            = bufferUsages;
        beginInfo.pInheritanceInfo = nullptr;
        /* Note that, if the command buffer was already recorded once, then a call to vkBeginCommandBuffer will implicitly
         * reset it. It's not possible to append commands to a buffer at a later time
        */
        auto result = vkBeginCommandBuffer (cmdBuffer, &beginInfo);
        if (result != VK_SUCCESS)
            throw std::runtime_error ("Failed to begin cmd buffer recording");
    }

    void endCmdBufferRecording (const VkCommandBuffer cmdBuffer) {
        auto result = vkEndCommandBuffer (cmdBuffer);
        if (result != VK_SUCCESS)
            throw std::runtime_error ("Failed to end cmd buffer recording");
    }

    void submitCmdBuffers (const VkQueue queue,
                           const VkFence signalFence,
                           const std::vector <VkCommandBuffer>& cmdBuffers,
                           const std::vector <VkSemaphore>& waitSemaphores,
                           const std::vector <VkPipelineStageFlags>& waitStageMasks,
                           const std::vector <VkSemaphore>& signalSemaphores,
                           std::vector <VkSubmitInfo>& submitInfos) {

        VkSubmitInfo submitInfo;
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext                = nullptr;
        submitInfo.commandBufferCount   = static_cast <uint32_t> (cmdBuffers.size());
        submitInfo.pCommandBuffers      = cmdBuffers.data();
        /* Note that, each entry in the wait stages array corresponds to the semaphore with the same index in the wait
         * semaphores array
        */
        submitInfo.waitSemaphoreCount   = static_cast <uint32_t> (waitSemaphores.size());
        submitInfo.pWaitSemaphores      = waitSemaphores.data();
        submitInfo.pWaitDstStageMask    = waitStageMasks.data();
        submitInfo.signalSemaphoreCount = static_cast <uint32_t> (signalSemaphores.size());
        submitInfo.pSignalSemaphores    = signalSemaphores.data();
        /* The fence passed in will be signaled when the command buffers finish execution. This allows us to know when it
         * is safe for the command buffers to be reused
        */
        submitInfos.push_back       (submitInfo);
        auto result = vkQueueSubmit (queue,
                                     static_cast <uint32_t> (submitInfos.size()),
                                     submitInfos.data(),
                                     signalFence);
        if (result != VK_SUCCESS)
            throw std::runtime_error ("Failed to submit cmd buffer(s)");
    }
}   // namespace Renderer