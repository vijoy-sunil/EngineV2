#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <vulkan/vk_enum_string_helper.h>
#include "../Backend/Layer/LYInstanceBase.h"
#include "../Backend/Log/LGImpl.h"
#include "VKPhyDevice.h"
#include "VKLogDevice.h"
#include "VKHelper.h"

namespace Core {
    class VKBuffer: public Layer::LYInstanceBase {
        private:
            struct BufferInfo {
                struct Meta {
                    VkDeviceSize size;
                    VkBufferUsageFlags usages;
                    VkMemoryPropertyFlags memoryProperties;
                    std::vector <uint32_t> queueFamilyIndices;
                    void* mappedMemory;
                } meta;

                struct State {
                    bool memoryMappingDisabled;
                    bool logObjCreated;
                } state;

                struct Resource {
                    Log::LGImpl* logObj;
                    VKPhyDevice* phyDeviceObj;
                    VKLogDevice* logDeviceObj;
                    VkBuffer buffer;
                    VkDeviceMemory memory;
                } resource;
            } m_bufferInfo;

        public:
            VKBuffer (Log::LGImpl* logObj,
                      VKPhyDevice* phyDeviceObj,
                      VKLogDevice* logDeviceObj) {

                m_bufferInfo = {};

                if (logObj == nullptr) {
                    m_bufferInfo.resource.logObj     = new Log::LGImpl();
                    m_bufferInfo.state.logObjCreated = true;

                    m_bufferInfo.resource.logObj->initLogInfo ("Build/Log/Core", __FILE__);
                    LOG_WARNING (m_bufferInfo.resource.logObj) << NULL_LOGOBJ_MSG
                                                               << std::endl;
                }
                else {
                    m_bufferInfo.resource.logObj     = logObj;
                    m_bufferInfo.state.logObjCreated = false;
                }

                if (phyDeviceObj == nullptr || logDeviceObj == nullptr) {
                    LOG_ERROR (m_bufferInfo.resource.logObj) << NULL_DEPOBJ_MSG
                                                             << std::endl;
                    throw std::runtime_error (NULL_DEPOBJ_MSG);
                }
                m_bufferInfo.resource.phyDeviceObj   = phyDeviceObj;
                m_bufferInfo.resource.logDeviceObj   = logDeviceObj;
            }

            void initBufferInfo (const VkDeviceSize size,
                                 const VkBufferUsageFlags bufferUsages,
                                 const VkMemoryPropertyFlags memoryProperties,
                                 const std::vector <uint32_t> queueFamilyIndices,
                                 const bool memoryMappingDisabled) {

                m_bufferInfo.meta.size                   = size;
                m_bufferInfo.meta.usages                 = bufferUsages;
                m_bufferInfo.meta.memoryProperties       = memoryProperties;
                m_bufferInfo.meta.queueFamilyIndices     = queueFamilyIndices;
                m_bufferInfo.meta.mappedMemory           = nullptr;
                m_bufferInfo.state.memoryMappingDisabled = memoryMappingDisabled;
                m_bufferInfo.resource.buffer             = nullptr;
                m_bufferInfo.resource.memory             = nullptr;
            }

            void updateBuffer (const void *data,
                               const bool persistentMappingDisabled) {

                memcpy (m_bufferInfo.meta.mappedMemory,
                        data,
                        static_cast <size_t> (m_bufferInfo.meta.size));
                /* Under normal scenarios, we map the buffer memory into CPU accessible memory with vkMapMemory. This
                 * allows us to access a region of the specified memory resource defined by an offset and size. We would
                 * then simply memcpy the desired data to the mapped memory and unmap it again using vkUnmapMemory. But,
                 * in some cases, the buffer stays mapped to this pointer for the application's whole lifetime. This
                 * technique is called "persistent mapping" and works on all Vulkan implementations. Not having to map
                 * the buffer every time we need to update it increases performances, as mapping is not free
                */
                if (persistentMappingDisabled)
                    vkUnmapMemory (*m_bufferInfo.resource.logDeviceObj->getLogDevice(),
                                    m_bufferInfo.resource.memory);
            }

            VkBuffer* getBuffer (void) {
                return &m_bufferInfo.resource.buffer;
            }

            void createBuffer (void) {
                VkBufferCreateInfo createInfo;
                createInfo.sType                     = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                createInfo.pNext                     = nullptr;
                createInfo.flags                     = 0;
                createInfo.size                      = m_bufferInfo.meta.size;
                createInfo.usage                     = m_bufferInfo.meta.usages;

                auto queueFamilyIndices              = m_bufferInfo.meta.queueFamilyIndices;
                /* If the queue families differ, then we'll be using the concurrent mode (buffers can be used across
                 * multiple queue families without explicit ownership transfers). Concurrent mode requires you to specify
                 * in advance between which queue families ownership will be shared using the queueFamilyIndexCount and
                 * pQueueFamilyIndices parameters
                */
                if (isIndicesUnique (queueFamilyIndices)) {
                    createInfo.sharingMode           = VK_SHARING_MODE_CONCURRENT;
                    createInfo.queueFamilyIndexCount = static_cast <uint32_t> (queueFamilyIndices.size());
                    createInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
                }
                /* If the queue families are the same, then we should stick to exclusive mode (a buffer is owned by one
                 * queue family at a time and ownership must be explicitly transferred before using it in another queue
                 * family. This option offers the best performance)
                */
                else {
                    createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
                    createInfo.queueFamilyIndexCount = 0;
                    createInfo.pQueueFamilyIndices   = nullptr;
                }

                auto result = vkCreateBuffer (*m_bufferInfo.resource.logDeviceObj->getLogDevice(),
                                               &createInfo,
                                               nullptr,
                                               &m_bufferInfo.resource.buffer);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_bufferInfo.resource.logObj) << "[?] Buffer"
                                                             << " "
                                                             << "[" << string_VkResult (result) << "]"
                                                             << std::endl;
                    throw std::runtime_error ("[?] Buffer");
                }
                LOG_INFO (m_bufferInfo.resource.logObj)      << "[O] Buffer"
                                                             << std::endl;

                /* The buffer has been created, but it doesn't actually have any memory assigned to it yet. Note that,
                 * before allocating memory for the buffer, we need to query its memory requirements
                */
                VkMemoryRequirements memoryRequirements;
                vkGetBufferMemoryRequirements (*m_bufferInfo.resource.logDeviceObj->getLogDevice(),
                                                m_bufferInfo.resource.buffer,
                                                &memoryRequirements);

                VkMemoryAllocateInfo allocInfo;
                allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.pNext           = nullptr;
                allocInfo.allocationSize  = memoryRequirements.size;
                allocInfo.memoryTypeIndex = getMemoryTypeIdx (*m_bufferInfo.resource.phyDeviceObj->getPhyDevice(),
                                                               memoryRequirements.memoryTypeBits,
                                                               m_bufferInfo.meta.memoryProperties);
                /* It should be noted that in a real world application, you're not supposed to actually call
                 * vkAllocateMemory for every individual buffer. The maximum number of simultaneous memory allocations
                 * is limited by the maxMemoryAllocationCount physical device limit, which may be as low as 4096 even
                 * on high end hardware like an NVIDIA GTX 1080. The right way to allocate memory for a large number of
                 * objects at the same time is to create a custom allocator that splits up a single allocation among
                 * many different objects by using the offset parameter
                */
                result = vkAllocateMemory (*m_bufferInfo.resource.logDeviceObj->getLogDevice(),
                                            &allocInfo,
                                            nullptr,
                                            &m_bufferInfo.resource.memory);
                if (result != VK_SUCCESS) {
                    LOG_ERROR (m_bufferInfo.resource.logObj) << "[?] Buffer memory"
                                                             << " "
                                                             << "[" << string_VkResult (result) << "]"
                                                             << std::endl;
                    throw std::runtime_error ("[?] Buffer memory");
                }
                LOG_INFO (m_bufferInfo.resource.logObj)      << "[O] Buffer memory"
                                                             << std::endl;

                /* If memory allocation was successful, then we can now associate this memory with the buffer */
                vkBindBufferMemory (*m_bufferInfo.resource.logDeviceObj->getLogDevice(),
                                     m_bufferInfo.resource.buffer,
                                     m_bufferInfo.resource.memory,
                                     0);

                if (!m_bufferInfo.state.memoryMappingDisabled)
                    vkMapMemory (*m_bufferInfo.resource.logDeviceObj->getLogDevice(),
                                  m_bufferInfo.resource.memory,
                                  0,
                                  m_bufferInfo.meta.size,
                                  0,
                                  &m_bufferInfo.meta.mappedMemory);
            }

            void destroyBuffer (void) {
                vkDestroyBuffer (*m_bufferInfo.resource.logDeviceObj->getLogDevice(),
                                  m_bufferInfo.resource.buffer,
                                  nullptr);
                LOG_INFO (m_bufferInfo.resource.logObj) << "[X] Buffer"
                                                        << std::endl;
                /* Memory that is bound to a buffer object may be freed once the buffer is no longer used, so we will
                 * free it after the buffer has been destroyed
                */
                vkFreeMemory    (*m_bufferInfo.resource.logDeviceObj->getLogDevice(),
                                  m_bufferInfo.resource.memory,
                                  nullptr);
                LOG_INFO (m_bufferInfo.resource.logObj) << "[X] Buffer memory"
                                                        << std::endl;
            }

            ~VKBuffer (void) {
                if (m_bufferInfo.state.logObjCreated)
                    delete m_bufferInfo.resource.logObj;
            }
    };
}   // namespace Core