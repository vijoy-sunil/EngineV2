#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>
#include "../../Backend/Scene/SNImpl.h"
#include "../../Backend/Collection/CNImpl.h"
#include "../../Backend/Log/LGImpl.h"
#include "../../Backend/Renderer/VKInstance.h"
#include "../../Backend/Renderer/VKWindow.h"
#include "../../Backend/Renderer/VKSurface.h"
#include "../../Backend/Renderer/VKPhyDevice.h"
#include "../../Backend/Renderer/VKLogDevice.h"
#include "../../Backend/Renderer/VKSwapChain.h"
#include "../../Backend/Renderer/VKBuffer.h"
#include "../../Backend/Renderer/VKImage.h"
#include "../../Backend/Renderer/VKSampler.h"
#include "../../Backend/Renderer/VKRenderPass.h"
#include "../../Backend/Renderer/VKFrameBuffer.h"
#include "../../Backend/Renderer/VKPipeline.h"
#include "../../Backend/Renderer/VKDescriptorPool.h"
#include "../../Backend/Renderer/VKDescriptorSet.h"
#include "../../Backend/Renderer/VKFence.h"
#include "../../Backend/Renderer/VKSemaphore.h"
#include "../../Backend/Renderer/VKCmdPool.h"
#include "../../Backend/Renderer/VKCmdBuffer.h"
#include "../../Backend/Renderer/VKRenderer.h"
#include "../System/SYMeshBatching.h"
#include "../System/SYMeshInstanceBatching.h"
#include "../System/SYLightInstanceBatching.h"
#include "../SBImpl.h"
#include "../../Backend/Renderer/VKCmdList.h"
#include "../../Backend/Renderer/VKHelper.h"
#include "../SBComponentType.h"
#include "../SBRendererType.h"

namespace SandBox {
    void SBImpl::configRenderer (void) {
        auto& skyBoxEntity          = m_sandBoxInfo.meta.skyBoxEntity;
        auto& sceneObj              = m_sandBoxInfo.resource.sceneObj;
        auto& collectionObj         = m_sandBoxInfo.resource.collectionObj;
        auto& defaultTexturePoolObj = m_sandBoxInfo.resource.defaultTexturePoolObj;
        auto& skyBoxTexturePoolObj  = m_sandBoxInfo.resource.skyBoxTexturePoolObj;

        {   /* Log              [DEFAULT] */
            collectionObj->registerCollectionType <Log::LGImpl>();

            auto logObj = new Log::LGImpl();
            logObj->initLogInfo (
                "Build/Log/Renderer",
                __FILE__
            );
            logObj->updateLogConfig (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_FILE);
            logObj->updateLogConfig (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
            logObj->updateLogConfig (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);

            collectionObj->addCollectionTypeInstance <Log::LGImpl> ("DEFAULT", logObj);
        }
        {   /* Instance         [DEFAULT] */
            collectionObj->registerCollectionType <Renderer::VKInstance>();

            auto logObj      = collectionObj->getCollectionTypeInstance <Log::LGImpl> ("DEFAULT");
            auto instanceObj = new Renderer::VKInstance (logObj);
            /* Vulkan does not come with any validation layers built-in, but the LunarG Vulkan SDK provides a nice set
             * of layers that check for common errors. Just like extensions, validation layers need to be enabled by
             * specifying their name. All of the useful standard validation is bundled into a layer included in the SDK
             * that is known as VK_LAYER_KHRONOS_validation
            */
            instanceObj->initInstanceInfo (
                {
                    "VK_LAYER_KHRONOS_validation"
                }
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKInstance> ("DEFAULT", instanceObj);
        }
        {   /* Window           [DEFAULT] */
            collectionObj->registerCollectionType <Renderer::VKWindow>();

            auto logObj    = collectionObj->getCollectionTypeInstance <Log::LGImpl> ("DEFAULT");
            auto windowObj = new Renderer::VKWindow (logObj);
            windowObj->initWindowInfo (
                1200,
                900,
                "EngineV2"
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKWindow> ("DEFAULT", windowObj);
            /* Set key event bindings */
            windowObj->toggleKeyEventCallback (
                true
            );
            windowObj->setKeyEventBinding (GLFW_KEY_ESCAPE,
                [windowObj](void) {
                    windowObj->toggleWindowClosed (true);
                }
            );
        }
        {   /* Surface          [DEFAULT] */
            collectionObj->registerCollectionType <Renderer::VKSurface>();

            auto logObj      = collectionObj->getCollectionTypeInstance <Log::LGImpl>          ("DEFAULT");
            auto instanceObj = collectionObj->getCollectionTypeInstance <Renderer::VKInstance> ("DEFAULT");
            auto windowObj   = collectionObj->getCollectionTypeInstance <Renderer::VKWindow>   ("DEFAULT");
            auto surfaceObj  = new Renderer::VKSurface (logObj, instanceObj, windowObj);
            surfaceObj->initSurfaceInfo();

            collectionObj->addCollectionTypeInstance <Renderer::VKSurface> ("DEFAULT", surfaceObj);
        }
        {   /* Phy device       [DEFAULT] */
            collectionObj->registerCollectionType <Renderer::VKPhyDevice>();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>          ("DEFAULT");
            auto instanceObj  = collectionObj->getCollectionTypeInstance <Renderer::VKInstance> ("DEFAULT");
            auto surfaceObj   = collectionObj->getCollectionTypeInstance <Renderer::VKSurface>  ("DEFAULT");
            auto phyDeviceObj = new Renderer::VKPhyDevice (logObj, instanceObj, surfaceObj);
            phyDeviceObj->initPhyDeviceInfo (
                {
#if __APPLE__
                    "VK_KHR_portability_subset",
#endif  // __APPLE__
                    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                    VK_KHR_MAINTENANCE_3_EXTENSION_NAME
                }
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT", phyDeviceObj);
        }
        {   /* Log device       [DEFAULT] */
            collectionObj->registerCollectionType <Renderer::VKLogDevice>();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto instanceObj  = collectionObj->getCollectionTypeInstance <Renderer::VKInstance>  ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = new Renderer::VKLogDevice (logObj, instanceObj, phyDeviceObj);
            logDeviceObj->initLogDeviceInfo();

            collectionObj->addCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT", logDeviceObj);
        }
        {   /* Swap chain       [DEFAULT] */
            collectionObj->registerCollectionType <Renderer::VKSwapChain>();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto windowObj    = collectionObj->getCollectionTypeInstance <Renderer::VKWindow>    ("DEFAULT");
            auto surfaceObj   = collectionObj->getCollectionTypeInstance <Renderer::VKSurface>   ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto swapChainObj = new Renderer::VKSwapChain (logObj, windowObj, surfaceObj, phyDeviceObj, logDeviceObj);
            /* Specify what kind of operations we'll use the images in the swap chain for. If we're going to render
             * directly to them, they're used as color attachment. It is also possible that you'll render images to a
             * separate image first to perform operations like post-processing. In that case you may use a value like
             * VK_IMAGE_USAGE_TRANSFER_DST_BIT instead and use a memory operation to transfer the rendered image to a
             * swap chain image
            */
            swapChainObj->initSwapChainInfo (
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                {
                    VK_FORMAT_B8G8R8A8_SRGB,
                    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
                },
                VK_PRESENT_MODE_MAILBOX_KHR,
                {
                    phyDeviceObj->getGraphicsQueueFamilyIdx(),
                    phyDeviceObj->getPresentQueueFamilyIdx()
                }
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKSwapChain> ("DEFAULT", swapChainObj);
        }
        {   /* Buffer           [DEFAULT_VERTEX_STAGING] */
            collectionObj->registerCollectionType <Renderer::VKBuffer>();

            auto batchingObj  = sceneObj->getSystem <SYMeshBatching>();
            auto vertices     = batchingObj->getBatchedVertices();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto bufferObj    = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
            bufferObj->initBufferInfo (
                vertices.size() * sizeof (Vertex),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                {
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                false
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("DEFAULT_VERTEX_STAGING", bufferObj);
            bufferObj->updateBuffer (
                vertices.data(),
                true
            );
        }
        {   /* Buffer           [SKY_BOX_VERTEX_STAGING] */
            auto meshComponent = sceneObj->getComponent <MeshComponent> (skyBoxEntity);
            auto vertices      = std::vector <glm::vec3> {};

            auto logObj        = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj  = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj  = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto bufferObj     = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);

            /* Repack vertex data since we only need the position as the vertex attribute */
            for (auto const& vertex: meshComponent->m_vertices)
                vertices.push_back (vertex.meta.position);

            bufferObj->initBufferInfo (
                vertices.size() * sizeof (vertices[0]),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                {
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                false
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("SKY_BOX_VERTEX_STAGING", bufferObj);
            bufferObj->updateBuffer (
                vertices.data(),
                true
            );
        }
        {   /* Buffer           [DEFAULT_VERTEX] */
            auto batchingObj  = sceneObj->getSystem <SYMeshBatching>();
            auto vertices     = batchingObj->getBatchedVertices();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto bufferObj    = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
            bufferObj->initBufferInfo (
                vertices.size() * sizeof (Vertex),
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                {
                    phyDeviceObj->getGraphicsQueueFamilyIdx(),
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                true
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("DEFAULT_VERTEX", bufferObj);
        }
        {   /* Buffer           [SKY_BOX_VERTEX] */
            auto meshComponent = sceneObj->getComponent <MeshComponent> (skyBoxEntity);
            auto vertices      = std::vector <glm::vec3> {};

            auto logObj        = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj  = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj  = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto bufferObj     = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);

            for (auto const& vertex: meshComponent->m_vertices)
                vertices.push_back (vertex.meta.position);

            bufferObj->initBufferInfo (
                vertices.size() * sizeof (vertices[0]),
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                {
                    phyDeviceObj->getGraphicsQueueFamilyIdx(),
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                true
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("SKY_BOX_VERTEX", bufferObj);
        }
        {   /* Buffer           [DEFAULT_INDEX_STAGING] */
            auto batchingObj  = sceneObj->getSystem <SYMeshBatching>();
            auto indices      = batchingObj->getBatchedIndices();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto bufferObj    = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
            bufferObj->initBufferInfo (
                indices.size() * sizeof (IndexType),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                {
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                false
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("DEFAULT_INDEX_STAGING", bufferObj);
            bufferObj->updateBuffer (
                indices.data(),
                true
            );
        }
        {   /* Buffer           [SKY_BOX_INDEX_STAGING] */
            auto meshComponent = sceneObj->getComponent <MeshComponent> (skyBoxEntity);
            auto indices       = meshComponent->m_indices;

            auto logObj        = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj  = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj  = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto bufferObj     = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
            bufferObj->initBufferInfo (
                indices.size() * sizeof (IndexType),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                {
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                false
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("SKY_BOX_INDEX_STAGING", bufferObj);
            bufferObj->updateBuffer (
                indices.data(),
                true
            );
        }
        {   /* Buffer           [DEFAULT_INDEX] */
            auto batchingObj  = sceneObj->getSystem <SYMeshBatching>();
            auto indices      = batchingObj->getBatchedIndices();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto bufferObj    = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
            bufferObj->initBufferInfo (
                indices.size() * sizeof (IndexType),
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                {
                    phyDeviceObj->getGraphicsQueueFamilyIdx(),
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                true
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("DEFAULT_INDEX", bufferObj);
        }
        {   /* Buffer           [SKY_BOX_INDEX] */
            auto meshComponent = sceneObj->getComponent <MeshComponent> (skyBoxEntity);
            auto indices       = meshComponent->m_indices;

            auto logObj        = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj  = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj  = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto bufferObj     = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
            bufferObj->initBufferInfo (
                indices.size() * sizeof (IndexType),
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                {
                    phyDeviceObj->getGraphicsQueueFamilyIdx(),
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                true
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("SKY_BOX_INDEX", bufferObj);
        }
        {   /* Buffer           [DEFAULT_TEXTURE_STAGING_?] */
            auto texturePool  = defaultTexturePoolObj->getTexturePool();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");

            for (auto const& [idx, info]: texturePool) {
                auto bufferObj = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
                bufferObj->initBufferInfo (
                    /* Note that, the image will be loaded with 4 channels due to the STBI_rgb_alpha flag */
                    static_cast <VkDeviceSize> (info.meta.width * info.meta.height * 4),
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    {
                        phyDeviceObj->getTransferQueueFamilyIdx()
                    },
                    false
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> (
                    "DEFAULT_TEXTURE_STAGING_" + std::to_string (idx),
                    bufferObj
                );
                bufferObj->updateBuffer (
                    info.resource.data,
                    true
                );
                defaultTexturePoolObj->destroyImage (idx);
            }
        }
        {   /* Buffer           [SKY_BOX_TEXTURE_STAGING_?] */
            auto texturePool  = skyBoxTexturePoolObj->getTexturePool();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");

            for (auto const& [idx, info]: texturePool) {
                auto bufferObj = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
                bufferObj->initBufferInfo (
                    static_cast <VkDeviceSize> (info.meta.width * info.meta.height * 4),
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    {
                        phyDeviceObj->getTransferQueueFamilyIdx()
                    },
                    false
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> (
                    "SKY_BOX_TEXTURE_STAGING_" + std::to_string (idx),
                    bufferObj
                );
                bufferObj->updateBuffer (
                    info.resource.data,
                    true
                );
                skyBoxTexturePoolObj->destroyImage (idx);
            }
        }
        {   /* Buffer           [DEFAULT_MESH_INSTANCE_?] */
            auto batchingObj  = sceneObj->getSystem <SYMeshInstanceBatching>();
            auto instances    = batchingObj->getBatchedMeshInstances();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            /* We should have multiple buffers, because multiple frames may be in flight at the same time and we don't
             * want to update the buffer in preparation of the next frame while a previous one is still reading from it.
             * Thus, we need to have as many buffers as we have frames in flight, and write to a buffer that is not
             * currently being read by the GPU
            */
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                auto bufferObj = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
                bufferObj->initBufferInfo (
                    instances.size() * sizeof (instances[0]),
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    {
                        phyDeviceObj->getGraphicsQueueFamilyIdx()
                    },
                    false
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> (
                    "DEFAULT_MESH_INSTANCE_" + std::to_string (i),
                    bufferObj
                );
            }
        }
        {   /* Buffer           [SKY_BOX_MESH_INSTANCE_?] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                auto bufferObj = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
                bufferObj->initBufferInfo (
                    sizeof (MeshInstanceUBO),
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    {
                        phyDeviceObj->getGraphicsQueueFamilyIdx()
                    },
                    false
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> (
                    "SKY_BOX_MESH_INSTANCE_" + std::to_string (i),
                    bufferObj
                );
            }
        }
        {   /* Buffer           [DEFAULT_LIGHT_INSTANCE_?] */
            auto batchingObj  = sceneObj->getSystem <SYLightInstanceBatching>();
            auto instances    = batchingObj->getBatchedLightInstances();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                auto bufferObj = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
                bufferObj->initBufferInfo (
                    instances.size() * sizeof (instances[0]),
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    {
                        phyDeviceObj->getGraphicsQueueFamilyIdx()
                    },
                    false
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> (
                    "DEFAULT_LIGHT_INSTANCE_" + std::to_string (i),
                    bufferObj
                );
            }
        }
        {   /* Image            [DEFAULT_COLOR] */
            collectionObj->registerCollectionType <Renderer::VKImage>();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto swapChainObj = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain> ("DEFAULT");
            auto imageObj     = new Renderer::VKImage (logObj, phyDeviceObj, logDeviceObj);
            /* Lazy allocation in Vulkan
             * Consider an example of deferred rendering. You need g-buffers. But you're going to fill them up during
             * the g-buffer pass, and you'll consume them during the lighting pass(es). After that point, you won't be
             * using their contents again. For many renderers, this doesn't really matter. But with a tile-based
             * renderer, it can. Why? Because if a tile is big enough to store all of the g-buffer data all at once,
             * then the implementation doesn't actually need to write the g-buffer data out to memory. It can just
             * leave everything in tile memory, do the lighting pass(es) within the tile (you read them as input
             * attachments), and then forget they exist
             *
             * But Vulkan requires that images have memory bound to them before they can be used. Lazy memory exists
             * so that you can fulfill that requirement while letting the implementation know that you aren't really
             * going to use this memory. Or more to the point, actual memory will only be allocated if you do something
             * that requires it
             *
             * Depth buffers and depth/stencil buffers could be lazily allocated too, so long as you don't need to
             * access them like regular images. Note that, it's not about a way to make stenciling or depth testing
             * optional. It's about making their backing storage ephemeral, memory that can live within a TBR's tile
             * and nowhere else. You're still doing the operations; it's just not taking up actual memory
             *
             * VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT flag specifies that implementations may support using memory
             * allocations with the VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT to back an image with this usage. This
             * bit can be set for any image that can be used to create a VkImageView suitable for use as a color,
             * resolve, depth/stencil, or input attachment
            */
            imageObj->initImageInfo (
                swapChainObj->getSwapChainExtent()->width,
                swapChainObj->getSwapChainExtent()->height,
                1,
                1,
                0,
                VK_IMAGE_LAYOUT_UNDEFINED,
                swapChainObj->getSwapChainFormat(),
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                Renderer::getMaxSupportedSamplesCount (*phyDeviceObj->getPhyDevice()),
                VK_IMAGE_TILING_OPTIMAL,
                VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
                {
                    phyDeviceObj->getGraphicsQueueFamilyIdx()
                },
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_VIEW_TYPE_2D
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKImage> ("DEFAULT_COLOR", imageObj);
        }
        {   /* Image            [DEFAULT_DEPTH] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto swapChainObj = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain> ("DEFAULT");
            auto imageObj     = new Renderer::VKImage (logObj, phyDeviceObj, logDeviceObj);
            auto imageFormat  = Renderer::getSupportedFormat (
                *phyDeviceObj->getPhyDevice(),
                {
                    VK_FORMAT_D32_SFLOAT,
                    VK_FORMAT_D32_SFLOAT_S8_UINT,
                    VK_FORMAT_D24_UNORM_S8_UINT
                },
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_IMAGE_TILING_OPTIMAL
            );
            imageObj->initImageInfo (
                swapChainObj->getSwapChainExtent()->width,
                swapChainObj->getSwapChainExtent()->height,
                1,
                1,
                0,
                VK_IMAGE_LAYOUT_UNDEFINED,
                imageFormat,
                VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                Renderer::getMaxSupportedSamplesCount (*phyDeviceObj->getPhyDevice()),
                VK_IMAGE_TILING_OPTIMAL,
                VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
                {
                    phyDeviceObj->getGraphicsQueueFamilyIdx()
                },
                VK_IMAGE_ASPECT_DEPTH_BIT,
                VK_IMAGE_VIEW_TYPE_2D
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKImage> ("DEFAULT_DEPTH", imageObj);
        }
        {   /* Image            [DEFAULT_SWAP_CHAIN_?] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto swapChainObj = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain> ("DEFAULT");
            auto images       = swapChainObj->getSwapChainImages();
            uint32_t loopIdx  = 0;

            for (auto const& image: images) {
                auto imageObj = new Renderer::VKImage (logObj, phyDeviceObj, logDeviceObj);
                imageObj->initImageInfo (
                    1,
                    1,
                    swapChainObj->getSwapChainFormat(),
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_VIEW_TYPE_2D,
                    image
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKImage> (
                    "DEFAULT_SWAP_CHAIN_" + std::to_string (loopIdx++),
                    imageObj
                );
            }
        }
        {   /* Image            [DEFAULT_TEXTURE_?] */
            auto texturePool  = defaultTexturePoolObj->getTexturePool();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");

            for (auto const& [idx, info]: texturePool) {
                auto imageObj = new Renderer::VKImage (logObj, phyDeviceObj, logDeviceObj);
                /* To calculate the number of levels in the mip chain, we use the max function to select the largest
                 * dimension. The log2 function calculates how many times that dimension can be divided by 2. The floor
                 * function handles cases where the largest dimension is not a power of 2. Finally, 1 is added so that
                 * the original image has a mip level
                */
                uint32_t mipLevels = static_cast <uint32_t> (
                    std::floor (std::log2 (std::max (
                        info.meta.width,
                        info.meta.height
                    )))
                ) + 1;
                auto imageFormat = Renderer::getSupportedFormat (
                    *phyDeviceObj->getPhyDevice(),
                    {
                        VK_FORMAT_R8G8B8A8_SRGB
                    },
                    VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
                    VK_IMAGE_TILING_OPTIMAL
                );
                imageObj->initImageInfo (
                    static_cast <uint32_t> (info.meta.width),
                    static_cast <uint32_t> (info.meta.height),
                    mipLevels,
                    1,
                    0,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    imageFormat,
                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    {
                        phyDeviceObj->getGraphicsQueueFamilyIdx(),
                        phyDeviceObj->getTransferQueueFamilyIdx()
                    },
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_VIEW_TYPE_2D
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKImage> (
                    "DEFAULT_TEXTURE_" + std::to_string (idx),
                    imageObj
                );
            }
        }
        {   /* Image            [SKY_BOX_TEXTURE] */
            auto texturePool  = skyBoxTexturePoolObj->getTexturePool();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto imageObj     = new Renderer::VKImage (logObj, phyDeviceObj, logDeviceObj);
            auto imageFormat  = Renderer::getSupportedFormat (
                *phyDeviceObj->getPhyDevice(),
                {
                    VK_FORMAT_R8G8B8A8_SRGB
                },
                VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
                VK_IMAGE_TILING_OPTIMAL
            );
            imageObj->initImageInfo (
                static_cast <uint32_t> (texturePool[0].meta.width),
                static_cast <uint32_t> (texturePool[0].meta.height),
                1,
                6,
                VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                imageFormat,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_TILING_OPTIMAL,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                {
                    phyDeviceObj->getGraphicsQueueFamilyIdx(),
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_VIEW_TYPE_CUBE
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKImage> ("SKY_BOX_TEXTURE", imageObj);
        }
        {   /* Sampler          [DEFAULT] */
            collectionObj->registerCollectionType <Renderer::VKSampler>();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto samplerObj   = new Renderer::VKSampler (logObj, logDeviceObj);
            /* It is recommended that to sample from the entire mip map chain, set min lod to 0.0, and set max lod to a
             * level of detail high enough that the computed level of detail will never be clamped. Assuming the standard
             * approach of halving the dimensions of an image for each mip level, a max lod of 13.0 would be appropriate
             * for a 4096x4096 source image
            */
            samplerObj->initSamplerInfo (
                0.0f,
                0.0f,
                13.0f,
                Renderer::getMaxSamplerAnisotropy (*phyDeviceObj->getPhyDevice()),
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VK_SAMPLER_MIPMAP_MODE_LINEAR,
                VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                VK_FALSE
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKSampler> ("DEFAULT", samplerObj);
        }
        {   /* Sampler          [SKY_BOX] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto samplerObj   = new Renderer::VKSampler (logObj, logDeviceObj);
            /* Note that, we are setting the sampler address mode to VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE since texture
             * coordinates that are exactly between two faces may not hit an exact face (due to hardware limitations) so
             * by using VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, the sampler always returns their edge values whenever we
             * sample between faces
            */
            samplerObj->initSamplerInfo (
                0.0f,
                0.0f,
                13.0f,
                Renderer::getMaxSamplerAnisotropy (*phyDeviceObj->getPhyDevice()),
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                VK_SAMPLER_MIPMAP_MODE_LINEAR,
                VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                VK_FALSE
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKSampler> ("SKY_BOX", samplerObj);
        }
        {   /* Render pass      [DEFAULT] */
            collectionObj->registerCollectionType <Renderer::VKRenderPass>();

            auto logObj        = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto logDeviceObj  = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto swapChainObj  = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain> ("DEFAULT");
            auto colorImageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage>     ("DEFAULT_COLOR");
            auto depthImageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage>     ("DEFAULT_DEPTH");
            auto renderPassObj = new Renderer::VKRenderPass (logObj, logDeviceObj);
            renderPassObj->initRenderPassInfo();

            /* Color attachment */
            renderPassObj->addRenderPassAttachment (
                0,
                colorImageObj->getImageFormat(),
                colorImageObj->getImageSamplesCount(),
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            );
            /* Depth attachment */
            renderPassObj->addRenderPassAttachment (
                0,
                depthImageObj->getImageFormat(),
                depthImageObj->getImageSamplesCount(),
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            );
            /* Resolve attachment */
            renderPassObj->addRenderPassAttachment (
                0,
                swapChainObj->getSwapChainFormat(),
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            );
            /* Sub pass */
            auto inputAttachmentReferences   = std::vector <VkAttachmentReference> {};
            /* The first transition for an attached image of a render pass will be from the initial layout for the
             * render pass to the reference layout for the first sub pass that uses the image. The last transition
             * for an attached image will be from reference layout of the final sub pass that uses the attachment to
             * the final layout for the render pass
            */
            auto colorAttachmentReferences   = std::vector {
                renderPassObj->createAttachmentReference (
                    0,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                )
            };
            auto depthAttachmentReference    =
                renderPassObj->createAttachmentReference (
                    1,
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                );
            auto resolveAttachmentReferences = std::vector {
                renderPassObj->createAttachmentReference (
                    2,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                )
            };
            renderPassObj->addSubPass (
                inputAttachmentReferences,
                colorAttachmentReferences,
                &depthAttachmentReference,
                resolveAttachmentReferences
            );
            /* Dependencies
             * It is possible that multiple frames are rendered simultaneously by the GPU. This is a problem when using
             * a single depth buffer, because one frame could overwrite the depth buffer while a previous frame is still
             * rendering to it. To prevent this, we add a sub pass dependency that synchronizes accesses to the depth
             * attachment. This dependency tells vulkan that the depth attachment in a render pass cannot be used before
             * previous render passes have finished using it
            */
            renderPassObj->addSubPassDependency (
                0,
                VK_SUBPASS_EXTERNAL,
                0,
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                VK_ACCESS_NONE,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
            );
            /* Before the render pass, the layout of the image will be transitioned to the layout you specify, for
             * example VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL. However, by default this happens at the beginning of
             * the pipeline at which point we haven't acquired the image yet (we acquire it at a later stage based on
             * our implementation, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT stage). That means that we need to
             * change the behaviour of the render pass to only change the layout once we've come to that stage
            */
            renderPassObj->addSubPassDependency (
                0,
                VK_SUBPASS_EXTERNAL,
                0,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_NONE,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKRenderPass> ("DEFAULT", renderPassObj);
        }
        {   /* Frame buffer     [DEFAULT_?] */
            collectionObj->registerCollectionType <Renderer::VKFrameBuffer>();

            auto logObj        = collectionObj->getCollectionTypeInstance <Log::LGImpl>            ("DEFAULT");
            auto logDeviceObj  = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice>  ("DEFAULT");
            auto swapChainObj  = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain>  ("DEFAULT");
            auto colorImageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage>      ("DEFAULT_COLOR");
            auto depthImageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage>      ("DEFAULT_DEPTH");
            auto renderPassObj = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass> ("DEFAULT");

            for (uint32_t i = 0; i < swapChainObj->getSwapChainImagesCount(); i++) {
                auto swapChainImageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage> (
                    "DEFAULT_SWAP_CHAIN_" + std::to_string (i)
                );

                auto bufferObj = new Renderer::VKFrameBuffer (logObj, logDeviceObj, renderPassObj);
                bufferObj->initFrameBufferInfo (
                    swapChainObj->getSwapChainExtent()->width,
                    swapChainObj->getSwapChainExtent()->height,
                    1,
                    {
                        *colorImageObj->getImageView(),     /* Attachment idx 0 */
                        *depthImageObj->getImageView(),     /* Attachment idx 1 */
                        *swapChainImageObj->getImageView()  /* Attachment idx 2 */
                    }
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKFrameBuffer> (
                    "DEFAULT_" + std::to_string (i),
                    bufferObj
                );
            }
        }
        {   /* Pipeline         [DEFAULT] */
            collectionObj->registerCollectionType <Renderer::VKPipeline>();

            auto texturePool   = defaultTexturePoolObj->getTexturePool();

            auto logObj        = collectionObj->getCollectionTypeInstance <Log::LGImpl>            ("DEFAULT");
            auto phyDeviceObj  = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice>  ("DEFAULT");
            auto logDeviceObj  = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice>  ("DEFAULT");
            auto renderPassObj = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass> ("DEFAULT");
            auto pipelineObj   = new Renderer::VKPipeline (logObj, logDeviceObj, renderPassObj);
            /* The allow derivative flag specifies that the pipeline to be created is allowed to be the parent of a
             * pipeline that will be created in a subsequent pipeline creation call. Pipeline derivatives can be used
             * for pipelines that share most of their state, depending on the implementation this may result in better
             * performance for pipeline switching and faster creation time
            */
            pipelineObj->initPipelineInfo (
                VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
                0,
                -1,
                nullptr
            );

            /* Vertex input */
            auto vertexBindingDescriptions   = std::vector {
                pipelineObj->createVertexBindingDescription (
                    0,
                    sizeof (Vertex),
                    VK_VERTEX_INPUT_RATE_VERTEX
                )
            };
            auto vertexAttributeDescriptions = std::vector {
                pipelineObj->createVertexAttributeDescription (
                    0,
                    0,
                    offsetof (Vertex, meta.uv),
                    VK_FORMAT_R32G32_SFLOAT
                ),
                pipelineObj->createVertexAttributeDescription (
                    0,
                    1,
                    offsetof (Vertex, meta.normal),
                    VK_FORMAT_R32G32B32_SFLOAT
                ),
                pipelineObj->createVertexAttributeDescription (
                    0,
                    2,
                    offsetof (Vertex, meta.position),
                    VK_FORMAT_R32G32B32_SFLOAT
                ),
                pipelineObj->createVertexAttributeDescription (
                    0,
                    3,
                    offsetof (Vertex, material.diffuseTextureIdx),
                    VK_FORMAT_R8_UINT
                ),
                pipelineObj->createVertexAttributeDescription (
                    0,
                    4,
                    offsetof (Vertex, material.specularTextureIdx),
                    VK_FORMAT_R8_UINT
                ),
                pipelineObj->createVertexAttributeDescription (
                    0,
                    5,
                    offsetof (Vertex, material.emissionTextureIdx),
                    VK_FORMAT_R8_UINT
                ),
                pipelineObj->createVertexAttributeDescription (
                    0,
                    6,
                    offsetof (Vertex, material.shininess),
                    VK_FORMAT_R32_UINT
                )
            };
            pipelineObj->createVertexInputState (
                vertexBindingDescriptions,
                vertexAttributeDescriptions
            );
            /* Input assembly */
            pipelineObj->createInputAssemblyState (
                VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                VK_FALSE
            );
            /* Shader stage */
            pipelineObj->addShaderStage (
                VK_SHADER_STAGE_VERTEX_BIT,
                "Build/Bin/Default[VERT].spv",
                "main"
            );
            pipelineObj->addShaderStage (
                VK_SHADER_STAGE_FRAGMENT_BIT,
                "Build/Bin/Default[FRAG].spv",
                "main"
            );
            /* Depth stencil */
            pipelineObj->createDepthStencilState (
                VK_TRUE,
                VK_TRUE,
                VK_FALSE,
                VK_FALSE,
                VK_COMPARE_OP_LESS,
                0.0f,
                1.0f,
                {},
                {}
            );
            /* Rasterization */
            pipelineObj->createRasterizationState (
                1.0f,
                VK_POLYGON_MODE_FILL,
                VK_CULL_MODE_NONE,
                VK_FRONT_FACE_COUNTER_CLOCKWISE
            );
            /* Multi sample */
            pipelineObj->createMultiSampleState (
                Renderer::getMaxSupportedSamplesCount (*phyDeviceObj->getPhyDevice()),
                VK_TRUE,
                0.2f
            );
            /* The most common way to use color blending is to implement alpha blending, where we want the new color to
             * be blended with the old color based on its opacity
             * finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor
             * finalColor.a   = newAlpha.a
            */
            auto colorBlendAttachments = std::vector {
                pipelineObj->createColorBlendAttachment (
                    VK_TRUE,
                    VK_BLEND_FACTOR_SRC_ALPHA,
                    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                    VK_BLEND_OP_ADD,
                    VK_BLEND_FACTOR_ONE,
                    VK_BLEND_FACTOR_ZERO,
                    VK_BLEND_OP_ADD,
                    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
                )
            };
            auto colorBlendConstants = std::vector <float> {
                0.0f,
                0.0f,
                0.0f,
                0.0f
            };
            pipelineObj->createColorBlendState (
                VK_FALSE,
                VK_LOGIC_OP_COPY,
                colorBlendAttachments,
                colorBlendConstants
            );
            /* Dynamic state */
            auto dynamicStates = std::vector {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };
            pipelineObj->createDynamicState (dynamicStates);
            /* View port */
            auto viewPorts = std::vector <VkViewport> {};
            auto scissors  = std::vector <VkRect2D>   {};
            pipelineObj->createViewPortState (
                viewPorts,
                scissors,
                1,
                1
            );
            /* Descriptor set layout */
            auto perFrameBindingFlags   = std::vector <VkDescriptorBindingFlags> {
                0,
                0
            };
            auto perFrameLayoutBindings = std::vector {
                pipelineObj->createDescriptorSetLayoutBinding (
                    0,
                    1,
                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    VK_SHADER_STAGE_VERTEX_BIT
                ),
                pipelineObj->createDescriptorSetLayoutBinding (
                    1,
                    1,
                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    VK_SHADER_STAGE_FRAGMENT_BIT
                )
            };
            /* Note that, if you look at the reported value of maxPerStageDescriptorSamplers, you may find it to be
             * too low for the application. This is because the report was run without argument buffer support turned
             * on. This is the maximum that metal supports passing directly to a shader function. With argument buffers,
             * (enabled by setting the MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS environment variable) the limit is much
             * higher. Using metal argument buffers dramatically increases the number of buffers, textures and samplers
             * that can be bound to a pipeline shader, and in most cases improves performance
             *
             * Additionally, we also have to enable and use VK_EXT_descriptor_indexing to get the validation layer to
             * look at maxPerStageDescriptorUpdateAfterBindSamplers instead of maxPerStageDescriptorSamplers. An
             * application needs to setup the following in order to achieve this
             *
             * (1) The VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT flag for any VkDescriptorSetLayout
             * the descriptor is from
             * (2) The VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT flag for any VkDescriptorPool the descriptor
             * is allocated from
             * (3) The VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT for each binding in the VkDescriptorSetLayout that
             * the descriptor will use
            */
            auto oneTimeBindingFlags   = std::vector <VkDescriptorBindingFlags> {
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT,
                0
            };
            auto oneTimeLayoutBindings = std::vector {
                pipelineObj->createDescriptorSetLayoutBinding (
                    0,
                    static_cast <uint32_t> (texturePool.size()),
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    VK_SHADER_STAGE_FRAGMENT_BIT
                ),
                pipelineObj->createDescriptorSetLayoutBinding (
                    1,
                    1,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    VK_SHADER_STAGE_FRAGMENT_BIT
                )
            };
            /* Layout 0 */
            pipelineObj->addDescriptorSetLayout (
                0,
                perFrameBindingFlags,
                perFrameLayoutBindings
            );
            /* Layout 1 */
            pipelineObj->addDescriptorSetLayout (
                VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
                oneTimeBindingFlags,
                oneTimeLayoutBindings
            );
            /* Push constant range */
            pipelineObj->addPushConstantRange (
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof (ActiveCameraPC)
            );
            pipelineObj->addPushConstantRange (
                VK_SHADER_STAGE_FRAGMENT_BIT,
                sizeof (ActiveCameraPC),
                sizeof (LightTypeOffsetsPC)
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKPipeline> ("DEFAULT", pipelineObj);
            pipelineObj->destroyShaderModules();
        }
        {   /* Pipeline         [SKY_BOX] */
            auto logObj          = collectionObj->getCollectionTypeInstance <Log::LGImpl>            ("DEFAULT");
            auto phyDeviceObj    = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice>  ("DEFAULT");
            auto logDeviceObj    = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice>  ("DEFAULT");
            auto renderPassObj   = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass> ("DEFAULT");
            auto basePipelineObj = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>   ("DEFAULT");
            auto pipelineObj     = new Renderer::VKPipeline (logObj, logDeviceObj, renderPassObj);
            pipelineObj->initPipelineInfo (
                VK_PIPELINE_CREATE_DERIVATIVE_BIT,
                0,
                -1,
                *basePipelineObj->getPipeline()
            );

            /* Vertex input */
            auto vertexBindingDescriptions   = std::vector {
                pipelineObj->createVertexBindingDescription (
                    0,
                    sizeof (glm::vec3),
                    VK_VERTEX_INPUT_RATE_VERTEX
                )
            };
            auto vertexAttributeDescriptions = std::vector {
                pipelineObj->createVertexAttributeDescription (
                    0,
                    0,
                    0,
                    VK_FORMAT_R32G32B32_SFLOAT
                )
            };
            pipelineObj->createVertexInputState (
                vertexBindingDescriptions,
                vertexAttributeDescriptions
            );
            /* Input assembly */
            pipelineObj->createInputAssemblyState (
                VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                VK_FALSE
            );
            /* Shader stage */
            pipelineObj->addShaderStage (
                VK_SHADER_STAGE_VERTEX_BIT,
                "Build/Bin/SkyBox[VERT].spv",
                "main"
            );
            pipelineObj->addShaderStage (
                VK_SHADER_STAGE_FRAGMENT_BIT,
                "Build/Bin/SkyBox[FRAG].spv",
                "main"
            );
            /* There are two options when we render the sky box. First, we render the sky box first before we render all
             * the other objects in the scene. This works great, but is too inefficient. If we render the sky box first,
             * we're running the fragment shader for each pixel on the screen even though only a small part of the sky
             * box will eventually be visible
             *
             * Second, render the sky box last. This way, the depth buffer is completely filled with all the scene's
             * depth values so we only have to render the sky box's fragments wherever the early depth test passes,
             * greatly reducing the number of fragment shader calls. The problem is that the sky box will most likely
             * render on top of all other objects since it's only a 1x1x1 cube, succeeding most depth tests
             *
             * We need to trick the depth buffer into believing that the sky box has the maximum depth value of 1.0 so
             * that it fails the depth test wherever there's a different object in front of it. We know that perspective
             * division is performed after the vertex shader has run (dividing the gl_Position's xyz coordinates by its
             * w component). We also know that the z component of the resulting division is equal to that vertex's depth
             * value. Using this information we can set the z component of the output position equal to its w component
             * which will result in a z component that is always equal to 1.0, because when the perspective division is
             * applied its z component translates to w / w = 1.0
             *
             * The resulting normalized device coordinates will then always have a z value equal to 1.0: the maximum
             * depth value. The sky box will as a result only be rendered wherever there are no objects visible
            */
            pipelineObj->createDepthStencilState (
                VK_TRUE,
                VK_TRUE,
                VK_FALSE,
                VK_FALSE,
                VK_COMPARE_OP_LESS_OR_EQUAL,
                0.0f,
                1.0f,
                {},
                {}
            );
            /* Rasterization */
            pipelineObj->createRasterizationState (
                1.0f,
                VK_POLYGON_MODE_FILL,
                VK_CULL_MODE_FRONT_BIT,
                VK_FRONT_FACE_COUNTER_CLOCKWISE
            );
            /* Multi sample */
            pipelineObj->createMultiSampleState (
                Renderer::getMaxSupportedSamplesCount (*phyDeviceObj->getPhyDevice()),
                VK_TRUE,
                0.2f
            );
            /* Color blend */
            auto colorBlendAttachments = std::vector {
                pipelineObj->createColorBlendAttachment (
                    VK_TRUE,
                    VK_BLEND_FACTOR_SRC_ALPHA,
                    VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                    VK_BLEND_OP_ADD,
                    VK_BLEND_FACTOR_ONE,
                    VK_BLEND_FACTOR_ZERO,
                    VK_BLEND_OP_ADD,
                    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                    VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
                )
            };
            auto colorBlendConstants = std::vector <float> {
                0.0f,
                0.0f,
                0.0f,
                0.0f
            };
            pipelineObj->createColorBlendState (
                VK_FALSE,
                VK_LOGIC_OP_COPY,
                colorBlendAttachments,
                colorBlendConstants
            );
            /* Dynamic state */
            auto dynamicStates = std::vector {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };
            pipelineObj->createDynamicState (dynamicStates);
            /* View port */
            auto viewPorts = std::vector <VkViewport> {};
            auto scissors  = std::vector <VkRect2D>   {};
            pipelineObj->createViewPortState (
                viewPorts,
                scissors,
                1,
                1
            );
            /* Descriptor set layout */
            auto perFrameBindingFlags   = std::vector <VkDescriptorBindingFlags> {
                0
            };
            auto perFrameLayoutBindings = std::vector {
                pipelineObj->createDescriptorSetLayoutBinding (
                    0,
                    1,
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    VK_SHADER_STAGE_VERTEX_BIT
                )
            };
            auto oneTimeBindingFlags    = std::vector <VkDescriptorBindingFlags> {
                0
            };
            auto oneTimeLayoutBindings  = std::vector {
                pipelineObj->createDescriptorSetLayoutBinding (
                    0,
                    1,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    VK_SHADER_STAGE_FRAGMENT_BIT
                )
            };
            /* Layout 0 */
            pipelineObj->addDescriptorSetLayout (
                0,
                perFrameBindingFlags,
                perFrameLayoutBindings
            );
            /* Layout 1 */
            pipelineObj->addDescriptorSetLayout (
                0,
                oneTimeBindingFlags,
                oneTimeLayoutBindings
            );
            /* Push constant range */
            pipelineObj->addPushConstantRange (
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof (ActiveCameraPC)
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKPipeline> ("SKY_BOX", pipelineObj);
            pipelineObj->destroyShaderModules();
        }
        {   /* Descriptor pool  [DEFAULT] */
            collectionObj->registerCollectionType <Renderer::VKDescriptorPool>();

            auto texturePool  = defaultTexturePoolObj->getTexturePool();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto descPoolObj  = new Renderer::VKDescriptorPool (logObj, logDeviceObj);
            descPoolObj->initDescriptorPoolInfo (
                VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
                g_maxFramesInFlight + 1
            );
            descPoolObj->addDescriptorPoolSize (
                g_maxFramesInFlight * 2,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
            );
            descPoolObj->addDescriptorPoolSize (
                static_cast <uint32_t> (texturePool.size()) + 1,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorPool> ("DEFAULT", descPoolObj);
        }
        {   /* Descriptor pool  [SKY_BOX] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto descPoolObj  = new Renderer::VKDescriptorPool (logObj, logDeviceObj);
            descPoolObj->initDescriptorPoolInfo (
                0,
                g_maxFramesInFlight + 1
            );
            descPoolObj->addDescriptorPoolSize (
                g_maxFramesInFlight,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
            );
            descPoolObj->addDescriptorPoolSize (
                1,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorPool> ("SKY_BOX", descPoolObj);
        }
        {   /* Descriptor sets  [DEFAULT_PER_FRAME] */
            collectionObj->registerCollectionType <Renderer::VKDescriptorSet>();

            auto meshInstanceBatchingObj  = sceneObj->getSystem <SYMeshInstanceBatching>();
            auto lightInstanceBatchingObj = sceneObj->getSystem <SYLightInstanceBatching>();
            auto meshInstances            = meshInstanceBatchingObj->getBatchedMeshInstances();
            auto lightInstances           = lightInstanceBatchingObj->getBatchedLightInstances();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>                ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice>      ("DEFAULT");
            auto pipelineObj  = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>       ("DEFAULT");
            auto descPoolObj  = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorPool> ("DEFAULT");
            auto descSetObj   = new Renderer::VKDescriptorSet (logObj, logDeviceObj, descPoolObj);
            descSetObj->initDescriptorSetInfo (
                g_maxFramesInFlight,
                pipelineObj->getDescriptorSetLayouts()[0]
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorSet> ("DEFAULT_PER_FRAME", descSetObj);

            typedef std::vector        <std::vector <VkDescriptorBufferInfo>> descriptorBufferInfosPool;
            auto descriptorImageInfos = std::vector <VkDescriptorImageInfo> {};
            std::unordered_map <uint32_t, descriptorBufferInfosPool> bindingToBufferInfosMap;
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                {   /* Binding 0 */
                    auto bufferObj             = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                        "DEFAULT_MESH_INSTANCE_" + std::to_string (i)
                    );
                    auto descriptorBufferInfos = std::vector <VkDescriptorBufferInfo> {
                        descSetObj->createDescriptorBufferInfo (
                            *bufferObj->getBuffer(),
                            0,
                            meshInstances.size() * sizeof (meshInstances[0])
                        )
                    };
                    bindingToBufferInfosMap[0].push_back (
                        descriptorBufferInfos
                    );
                }
                {   /* Binding 1 */
                    auto bufferObj             = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                        "DEFAULT_LIGHT_INSTANCE_" + std::to_string (i)
                    );
                    auto descriptorBufferInfos = std::vector <VkDescriptorBufferInfo> {
                        descSetObj->createDescriptorBufferInfo (
                            *bufferObj->getBuffer(),
                            0,
                            lightInstances.size() * sizeof (lightInstances[0])
                        )
                    };
                    bindingToBufferInfosMap[1].push_back (
                        descriptorBufferInfos
                    );
                }
            }
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                descSetObj->addWriteDescriptorSet (
                    0,
                    1,
                    0,
                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    descSetObj->getDescriptorSets()[i],
                    bindingToBufferInfosMap[0][i],
                    descriptorImageInfos
                );
                descSetObj->addWriteDescriptorSet (
                    1,
                    1,
                    0,
                    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    descSetObj->getDescriptorSets()[i],
                    bindingToBufferInfosMap[1][i],
                    descriptorImageInfos
                );
            }
            descSetObj->updateDescriptorSets();
        }
        {   /* Descriptor sets  [SKY_BOX_PER_FRAME] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>                ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice>      ("DEFAULT");
            auto pipelineObj  = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>       ("SKY_BOX");
            auto descPoolObj  = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorPool> ("SKY_BOX");
            auto descSetObj   = new Renderer::VKDescriptorSet (logObj, logDeviceObj, descPoolObj);
            descSetObj->initDescriptorSetInfo (
                g_maxFramesInFlight,
                pipelineObj->getDescriptorSetLayouts()[0]
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorSet> ("SKY_BOX_PER_FRAME", descSetObj);

            typedef std::vector        <std::vector <VkDescriptorBufferInfo>> descriptorBufferInfosPool;
            auto descriptorImageInfos = std::vector <VkDescriptorImageInfo> {};
            std::unordered_map <uint32_t, descriptorBufferInfosPool> bindingToBufferInfosMap;
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                {   /* Binding 0 */
                    auto bufferObj             = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                        "SKY_BOX_MESH_INSTANCE_" + std::to_string (i)
                    );
                    auto descriptorBufferInfos = std::vector <VkDescriptorBufferInfo> {
                        descSetObj->createDescriptorBufferInfo (
                            *bufferObj->getBuffer(),
                            0,
                            sizeof (MeshInstanceUBO)
                        )
                    };
                    bindingToBufferInfosMap[0].push_back (
                        descriptorBufferInfos
                    );
                }
            }
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                descSetObj->addWriteDescriptorSet (
                    0,
                    1,
                    0,
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    descSetObj->getDescriptorSets()[i],
                    bindingToBufferInfosMap[0][i],
                    descriptorImageInfos
                );
            }
            descSetObj->updateDescriptorSets();
        }
        {   /* Descriptor sets  [DEFAULT_ONE_TIME] */
            auto texturePool      = defaultTexturePoolObj->getTexturePool();

            auto logObj           = collectionObj->getCollectionTypeInstance <Log::LGImpl>                ("DEFAULT");
            auto logDeviceObj     = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice>      ("DEFAULT");
            auto imageObj         = collectionObj->getCollectionTypeInstance <Renderer::VKImage>          (
                "SKY_BOX_TEXTURE"
            );
            auto samplerObj       = collectionObj->getCollectionTypeInstance <Renderer::VKSampler>        ("DEFAULT");
            auto skyBoxSamplerObj = collectionObj->getCollectionTypeInstance <Renderer::VKSampler>        ("SKY_BOX");
            auto pipelineObj      = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>       ("DEFAULT");
            auto descPoolObj      = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorPool> ("DEFAULT");
            auto descSetObj       = new Renderer::VKDescriptorSet (logObj, logDeviceObj, descPoolObj);
            descSetObj->initDescriptorSetInfo (
                1,
                pipelineObj->getDescriptorSetLayouts()[1]
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorSet> ("DEFAULT_ONE_TIME", descSetObj);

            auto descriptorBufferInfos =  std::vector <VkDescriptorBufferInfo> {};
            std::unordered_map <uint32_t, std::vector <VkDescriptorImageInfo>> bindingToImageInfosMap;
            /* Binding 0 */
            for (auto const& [idx, info]: texturePool) {
                auto imageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage> (
                    "DEFAULT_TEXTURE_" + std::to_string (idx)
                );
                /* Note that, we can push the info to the vector since the texture pool is ordered by its indices */
                bindingToImageInfosMap[0].push_back (
                    descSetObj->createDescriptorImageInfo (
                        *samplerObj->getSampler(),
                        *imageObj->getImageView(),
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    )
                );
            }
            /* Binding 1 */
            bindingToImageInfosMap[1].push_back (
                descSetObj->createDescriptorImageInfo (
                    *skyBoxSamplerObj->getSampler(),
                    *imageObj->getImageView(),
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                )
            );
            descSetObj->addWriteDescriptorSet (
                0,
                static_cast <uint32_t> (texturePool.size()),
                0,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                descSetObj->getDescriptorSets()[0],
                descriptorBufferInfos,
                bindingToImageInfosMap[0]
            );
            descSetObj->addWriteDescriptorSet (
                1,
                1,
                0,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                descSetObj->getDescriptorSets()[0],
                descriptorBufferInfos,
                bindingToImageInfosMap[1]
            );
            descSetObj->updateDescriptorSets();
        }
        {   /* Descriptor sets  [SKY_BOX_ONE_TIME] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>                ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice>      ("DEFAULT");
            auto imageObj     = collectionObj->getCollectionTypeInstance <Renderer::VKImage>          ("SKY_BOX_TEXTURE");
            auto samplerObj   = collectionObj->getCollectionTypeInstance <Renderer::VKSampler>        ("SKY_BOX");
            auto pipelineObj  = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>       ("SKY_BOX");
            auto descPoolObj  = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorPool> ("SKY_BOX");
            auto descSetObj   = new Renderer::VKDescriptorSet (logObj, logDeviceObj, descPoolObj);
            descSetObj->initDescriptorSetInfo (
                1,
                pipelineObj->getDescriptorSetLayouts()[1]
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorSet> ("SKY_BOX_ONE_TIME", descSetObj);

            auto descriptorBufferInfos = std::vector <VkDescriptorBufferInfo> {};
            auto descriptorImageInfos  = std::vector {
                descSetObj->createDescriptorImageInfo (
                    *samplerObj->getSampler(),
                    *imageObj->getImageView(),
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                )
            };
            descSetObj->addWriteDescriptorSet (
                0,
                1,
                0,
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                descSetObj->getDescriptorSets()[0],
                descriptorBufferInfos,
                descriptorImageInfos
            );
            descSetObj->updateDescriptorSets();
        }
        {   /* Fence            [DEFAULT_COPY_OPS] */
            collectionObj->registerCollectionType <Renderer::VKFence>();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto fenObj       = new Renderer::VKFence (logObj, logDeviceObj);
            fenObj->initFenceInfo (
                0
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKFence> ("DEFAULT_COPY_OPS", fenObj);
        }
        {   /* Fence            [SKY_BOX_COPY_OPS] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto fenObj       = new Renderer::VKFence (logObj, logDeviceObj);
            fenObj->initFenceInfo (
                0
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKFence> ("SKY_BOX_COPY_OPS", fenObj);
        }
        {   /* Fence            [DEFAULT_BLIT_OPS] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto fenObj       = new Renderer::VKFence (logObj, logDeviceObj);
            fenObj->initFenceInfo (
                0
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKFence> ("DEFAULT_BLIT_OPS", fenObj);
        }
        {   /* Fence            [DEFAULT_IN_FLIGHT_?] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                auto fenObj = new Renderer::VKFence (logObj, logDeviceObj);
                /* On the very first frame, we immediately wait on in flight fence to be signaled. This fence is only
                 * signaled after a frame has finished rendering, yet since this is the first frame, there are no
                 * previous frames in which to signal the fence! Thus vkWaitForFences() blocks indefinitely, waiting
                 * on something which will never happen. To combat this, create the fence in the signaled state, so
                 * that the first call to vkWaitForFences() returns immediately since the fence is already signaled
                */
                fenObj->initFenceInfo (
                    VK_FENCE_CREATE_SIGNALED_BIT
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKFence> (
                    "DEFAULT_IN_FLIGHT_" + std::to_string (i),
                    fenObj
                );
            }
        }
        {   /* Semaphore        [DEFAULT_IMG_AVAILABLE_?] */
            collectionObj->registerCollectionType <Renderer::VKSemaphore>();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                auto semObj = new Renderer::VKSemaphore (logObj, logDeviceObj);
                semObj->initSemaphoreInfo (
                    0
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKSemaphore> (
                    "DEFAULT_IMG_AVAILABLE_" + std::to_string (i),
                    semObj
                );
            }
        }
        {   /* Semaphore        [DEFAULT_RENDER_DONE_?] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                auto semObj = new Renderer::VKSemaphore (logObj, logDeviceObj);
                semObj->initSemaphoreInfo (
                    0
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKSemaphore> (
                    "DEFAULT_RENDER_DONE_" + std::to_string (i),
                    semObj
                );
            }
        }
        {   /* Cmd pool         [DEFAULT_COPY_OPS] */
            collectionObj->registerCollectionType <Renderer::VKCmdPool>();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto cmdPoolObj   = new Renderer::VKCmdPool (logObj, logDeviceObj);
            /* Note that the command buffer that we will be allocating from this pool will be short lived, hence why
             * we will choose the VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag
            */
            cmdPoolObj->initCmdPoolInfo (
                VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                phyDeviceObj->getTransferQueueFamilyIdx()
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKCmdPool> ("DEFAULT_COPY_OPS", cmdPoolObj);
        }
        {   /* Cmd pool         [SKY_BOX_COPY_OPS] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto cmdPoolObj   = new Renderer::VKCmdPool (logObj, logDeviceObj);
            cmdPoolObj->initCmdPoolInfo (
                VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                phyDeviceObj->getTransferQueueFamilyIdx()
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKCmdPool> ("SKY_BOX_COPY_OPS", cmdPoolObj);
        }
        {   /* Cmd pool         [DEFAULT_BLIT_OPS] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto cmdPoolObj   = new Renderer::VKCmdPool (logObj, logDeviceObj);
            cmdPoolObj->initCmdPoolInfo (
                VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                phyDeviceObj->getGraphicsQueueFamilyIdx()
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKCmdPool> ("DEFAULT_BLIT_OPS", cmdPoolObj);
        }
        {   /* Cmd pool         [DEFAULT_DRAW_OPS] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto cmdPoolObj   = new Renderer::VKCmdPool (logObj, logDeviceObj);
            /* We will be recording a command buffer every frame, so we want to be able to reset and rerecord over
             * it, hence why we need the VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag
            */
            cmdPoolObj->initCmdPoolInfo (
                VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                phyDeviceObj->getGraphicsQueueFamilyIdx()
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKCmdPool> ("DEFAULT_DRAW_OPS", cmdPoolObj);
        }
        {   /* Cmd buffers      [DEFAULT_COPY_OPS] */
            collectionObj->registerCollectionType <Renderer::VKCmdBuffer>();

            auto texturePool  = defaultTexturePoolObj->getTexturePool();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto fenObj       = collectionObj->getCollectionTypeInstance <Renderer::VKFence>     ("DEFAULT_COPY_OPS");
            auto cmdPoolObj   = collectionObj->getCollectionTypeInstance <Renderer::VKCmdPool>   ("DEFAULT_COPY_OPS");
            auto bufferObj    = new Renderer::VKCmdBuffer (logObj, logDeviceObj, cmdPoolObj);
            /* Note that we are only requesting one command buffer from the pool, since it is recommended to combine
             * operations in a single command buffer and execute them asynchronously for higher throughput
            */
            bufferObj->initCmdBufferInfo (
                1,
                VK_COMMAND_BUFFER_LEVEL_PRIMARY
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKCmdBuffer> ("DEFAULT_COPY_OPS", bufferObj);
            /* We're only going to record the command buffer once and wait until the operations have finished executing.
             * It's good practice to tell the driver about our intent using VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
            */
            Renderer::beginCmdBufferRecording (
                bufferObj->getCmdBuffers()[0],
                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
            );
            {   /* Buffer->Buffer */
                auto srcBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "DEFAULT_VERTEX_STAGING"
                );
                auto dstBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "DEFAULT_VERTEX"
                );
                auto copyRegions  = std::vector <VkBufferCopy> {};
                Renderer::copyBufferToBuffer (
                    bufferObj->getCmdBuffers()[0],
                    *srcBufferObj->getBuffer(),
                    *dstBufferObj->getBuffer(),
                    0,
                    0,
                    srcBufferObj->getBufferSize(),
                    copyRegions
                );
            }
            {   /* Buffer->Buffer */
                auto srcBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "DEFAULT_INDEX_STAGING"
                );
                auto dstBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "DEFAULT_INDEX"
                );
                auto copyRegions  = std::vector <VkBufferCopy> {};
                Renderer::copyBufferToBuffer (
                    bufferObj->getCmdBuffers()[0],
                    *srcBufferObj->getBuffer(),
                    *dstBufferObj->getBuffer(),
                    0,
                    0,
                    srcBufferObj->getBufferSize(),
                    copyRegions
                );
            }
            {   /* Buffer->Image  */
                for (auto const& [idx, info]: texturePool) {
                    auto srcBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                        "DEFAULT_TEXTURE_STAGING_" + std::to_string (idx)
                    );
                    auto dstImageObj  = collectionObj->getCollectionTypeInstance <Renderer::VKImage> (
                        "DEFAULT_TEXTURE_"         + std::to_string (idx)
                    );
                    auto copyRegions  = std::vector <VkBufferImageCopy> {};
                    Renderer::copyBufferToImage (
                        bufferObj->getCmdBuffers()[0],
                        *srcBufferObj->getBuffer(),
                        *dstImageObj->getImage(),
                        0,
                        0,
                        0,
                        {0, 0, 0},
                        {
                            static_cast <uint32_t> (info.meta.width),
                            static_cast <uint32_t> (info.meta.height),
                            1
                        },
                        dstImageObj->getImageMipLevels(),
                        0,
                        1,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        dstImageObj->getImageAspectFlags(),
                        copyRegions
                    );
                }
            }
            Renderer::endCmdBufferRecording (
                bufferObj->getCmdBuffers()[0]
            );

            auto waitSemaphores   = std::vector <VkSemaphore> {};
            auto waitStageMasks   = std::vector <VkPipelineStageFlags> {};
            auto signalSemaphores = std::vector <VkSemaphore> {};
            auto cmdBuffers       = std::vector {
                bufferObj->getCmdBuffers()[0]
            };
            auto submitInfos      = std::vector <VkSubmitInfo> {};
            Renderer::submitCmdBuffers (
                *logDeviceObj->getTransferQueue(),
                *fenObj->getFence(),
                cmdBuffers,
                waitSemaphores,
                waitStageMasks,
                signalSemaphores,
                submitInfos
            );
            fenObj->waitForFence();
            fenObj->resetFence();
            /* Destroy copy ops temperory resources */
            collectionObj->removeCollectionTypeInstance <Renderer::VKCmdBuffer>  ("DEFAULT_COPY_OPS");
            collectionObj->removeCollectionTypeInstance <Renderer::VKCmdPool>    ("DEFAULT_COPY_OPS");
            collectionObj->removeCollectionTypeInstance <Renderer::VKFence>      ("DEFAULT_COPY_OPS");
            for (auto const& [idx, info]: texturePool)
                collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer> (
                    "DEFAULT_TEXTURE_STAGING_" + std::to_string (idx)
                );
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>     ("DEFAULT_INDEX_STAGING");
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>     ("DEFAULT_VERTEX_STAGING");
        }
        {   /* Cmd buffers      [SKY_BOX_COPY_OPS] */
            auto texturePool  = skyBoxTexturePoolObj->getTexturePool();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto fenObj       = collectionObj->getCollectionTypeInstance <Renderer::VKFence>     ("SKY_BOX_COPY_OPS");
            auto cmdPoolObj   = collectionObj->getCollectionTypeInstance <Renderer::VKCmdPool>   ("SKY_BOX_COPY_OPS");
            auto bufferObj    = new Renderer::VKCmdBuffer (logObj, logDeviceObj, cmdPoolObj);
            bufferObj->initCmdBufferInfo (
                1,
                VK_COMMAND_BUFFER_LEVEL_PRIMARY
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKCmdBuffer> ("SKY_BOX_COPY_OPS", bufferObj);

            Renderer::beginCmdBufferRecording (
                bufferObj->getCmdBuffers()[0],
                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
            );
            {   /* Buffer->Buffer */
                auto srcBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "SKY_BOX_VERTEX_STAGING"
                );
                auto dstBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "SKY_BOX_VERTEX"
                );
                auto copyRegions  = std::vector <VkBufferCopy> {};
                Renderer::copyBufferToBuffer (
                    bufferObj->getCmdBuffers()[0],
                    *srcBufferObj->getBuffer(),
                    *dstBufferObj->getBuffer(),
                    0,
                    0,
                    srcBufferObj->getBufferSize(),
                    copyRegions
                );
            }
            {   /* Buffer->Buffer */
                auto srcBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "SKY_BOX_INDEX_STAGING"
                );
                auto dstBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "SKY_BOX_INDEX"
                );
                auto copyRegions  = std::vector <VkBufferCopy> {};
                Renderer::copyBufferToBuffer (
                    bufferObj->getCmdBuffers()[0],
                    *srcBufferObj->getBuffer(),
                    *dstBufferObj->getBuffer(),
                    0,
                    0,
                    srcBufferObj->getBufferSize(),
                    copyRegions
                );
            }
            {   /* Buffer->Image  */
                for (auto const& [idx, info]: texturePool) {
                    auto srcBufferObj   = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                        "SKY_BOX_TEXTURE_STAGING_" + std::to_string (idx)
                    );
                    auto dstImageObj    = collectionObj->getCollectionTypeInstance <Renderer::VKImage> (
                        "SKY_BOX_TEXTURE"
                    );
                    auto copyRegions    = std::vector <VkBufferImageCopy> {};
                    auto appendBarriers = std::vector <VkImageMemoryBarrier> {};
                    Renderer::copyBufferToImage (
                        bufferObj->getCmdBuffers()[0],
                        *srcBufferObj->getBuffer(),
                        *dstImageObj->getImage(),
                        0,
                        0,
                        0,
                        {0, 0, 0},
                        {
                            static_cast <uint32_t> (info.meta.width),
                            static_cast <uint32_t> (info.meta.height),
                            1
                        },
                        dstImageObj->getImageMipLevels(),
                        idx,    /* Note that, we are using texture idx as layer idx here */
                        1,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        dstImageObj->getImageAspectFlags(),
                        copyRegions
                    );
                    Renderer::transitionImageLayout (
                        bufferObj->getCmdBuffers()[0],
                        *dstImageObj->getImage(),
                        0,
                        dstImageObj->getImageMipLevels(),
                        idx,    /* Note that, we are using texture idx as layer idx here */
                        1,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        dstImageObj->getImageAspectFlags(),
                        appendBarriers
                    );
                }
            }
            Renderer::endCmdBufferRecording (
                bufferObj->getCmdBuffers()[0]
            );

            auto waitSemaphores   = std::vector <VkSemaphore> {};
            auto waitStageMasks   = std::vector <VkPipelineStageFlags> {};
            auto signalSemaphores = std::vector <VkSemaphore> {};
            auto cmdBuffers       = std::vector {
                bufferObj->getCmdBuffers()[0]
            };
            auto submitInfos      = std::vector <VkSubmitInfo> {};
            Renderer::submitCmdBuffers (
                *logDeviceObj->getTransferQueue(),
                *fenObj->getFence(),
                cmdBuffers,
                waitSemaphores,
                waitStageMasks,
                signalSemaphores,
                submitInfos
            );
            fenObj->waitForFence();
            fenObj->resetFence();
            /* Destroy copy ops temperory resources */
            collectionObj->removeCollectionTypeInstance <Renderer::VKCmdBuffer>  ("SKY_BOX_COPY_OPS");
            collectionObj->removeCollectionTypeInstance <Renderer::VKCmdPool>    ("SKY_BOX_COPY_OPS");
            collectionObj->removeCollectionTypeInstance <Renderer::VKFence>      ("SKY_BOX_COPY_OPS");
            for (auto const& [idx, info]: texturePool)
                collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer> (
                    "SKY_BOX_TEXTURE_STAGING_" + std::to_string (idx)
                );
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>     ("SKY_BOX_INDEX_STAGING");
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>     ("SKY_BOX_VERTEX_STAGING");
        }
        {   /* Cmd buffers      [DEFAULT_BLIT_OPS] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto fenObj       = collectionObj->getCollectionTypeInstance <Renderer::VKFence>     ("DEFAULT_BLIT_OPS");
            auto cmdPoolObj   = collectionObj->getCollectionTypeInstance <Renderer::VKCmdPool>   ("DEFAULT_BLIT_OPS");
            auto bufferObj    = new Renderer::VKCmdBuffer (logObj, logDeviceObj, cmdPoolObj);
            bufferObj->initCmdBufferInfo (
                1,
                VK_COMMAND_BUFFER_LEVEL_PRIMARY
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKCmdBuffer> ("DEFAULT_BLIT_OPS", bufferObj);
            /* Record and submit ops */
            Renderer::beginCmdBufferRecording (
                bufferObj->getCmdBuffers()[0],
                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
            );
            {   /* Image->Image */
                auto texturePool = defaultTexturePoolObj->getTexturePool();

                for (auto const& [idx, info]: texturePool) {
                    auto imageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage> (
                        "DEFAULT_TEXTURE_" + std::to_string (idx)
                    );
                    Renderer::blitImageToMipMaps (
                        bufferObj->getCmdBuffers()[0],
                        *imageObj->getImage(),
                        static_cast <uint32_t> (info.meta.width),
                        static_cast <uint32_t> (info.meta.height),
                        imageObj->getImageMipLevels(),
                        0,
                        imageObj->getImageAspectFlags()
                    );
                }
            }
            Renderer::endCmdBufferRecording (
                bufferObj->getCmdBuffers()[0]
            );

            auto waitSemaphores   = std::vector <VkSemaphore> {};
            auto waitStageMasks   = std::vector <VkPipelineStageFlags> {};
            auto signalSemaphores = std::vector <VkSemaphore> {};
            auto cmdBuffers       = std::vector {
                bufferObj->getCmdBuffers()[0]
            };
            auto submitInfos      = std::vector <VkSubmitInfo> {};
            Renderer::submitCmdBuffers (
                *logDeviceObj->getGraphicsQueue(),
                *fenObj->getFence(),
                cmdBuffers,
                waitSemaphores,
                waitStageMasks,
                signalSemaphores,
                submitInfos
            );
            fenObj->waitForFence();
            fenObj->resetFence();
            /* Destroy blit ops temperory resources */
            collectionObj->removeCollectionTypeInstance <Renderer::VKCmdBuffer> ("DEFAULT_BLIT_OPS");
            collectionObj->removeCollectionTypeInstance <Renderer::VKCmdPool>   ("DEFAULT_BLIT_OPS");
            collectionObj->removeCollectionTypeInstance <Renderer::VKFence>     ("DEFAULT_BLIT_OPS");
        }
        {   /* Cmd buffers      [DEFAULT_DRAW_OPS] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto cmdPoolObj   = collectionObj->getCollectionTypeInstance <Renderer::VKCmdPool>   ("DEFAULT_DRAW_OPS");
            auto bufferObj    = new Renderer::VKCmdBuffer (logObj, logDeviceObj, cmdPoolObj);
            bufferObj->initCmdBufferInfo (
                g_maxFramesInFlight,
                VK_COMMAND_BUFFER_LEVEL_PRIMARY
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKCmdBuffer> ("DEFAULT_DRAW_OPS", bufferObj);
        }
        {   /* Renderer         [DEFAULT] */
            collectionObj->registerCollectionType <Renderer::VKRenderer>();

            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("DEFAULT");
            auto windowObj    = collectionObj->getCollectionTypeInstance <Renderer::VKWindow>    ("DEFAULT");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
            auto swapChainObj = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain> ("DEFAULT");
            auto cmdBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKCmdBuffer> ("DEFAULT_DRAW_OPS");

            auto inFlightFenObjs          = std::vector <Renderer::VKFence*>     {};
            auto imageAvailableSemObjs    = std::vector <Renderer::VKSemaphore*> {};
            auto renderDoneSemObjs        = std::vector <Renderer::VKSemaphore*> {};
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                auto inFlightFenObj       = collectionObj->getCollectionTypeInstance <Renderer::VKFence> (
                    "DEFAULT_IN_FLIGHT_"     + std::to_string (i)
                );
                auto imageAvailableSemObj = collectionObj->getCollectionTypeInstance <Renderer::VKSemaphore> (
                    "DEFAULT_IMG_AVAILABLE_" + std::to_string (i)
                );
                auto renderDoneSemObj     = collectionObj->getCollectionTypeInstance <Renderer::VKSemaphore> (
                    "DEFAULT_RENDER_DONE_"   + std::to_string (i)
                );
                inFlightFenObjs.push_back       (inFlightFenObj);
                imageAvailableSemObjs.push_back (imageAvailableSemObj);
                renderDoneSemObjs.push_back     (renderDoneSemObj);
            };
            auto rendererObj = new Renderer::VKRenderer (
                logObj,
                windowObj,
                logDeviceObj,
                swapChainObj,
                inFlightFenObjs,
                imageAvailableSemObjs,
                renderDoneSemObjs,
                cmdBufferObj
            );
            rendererObj->initRendererInfo (
                g_maxFramesInFlight
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKRenderer> ("DEFAULT", rendererObj);
            /* View port resize bindings */
            rendererObj->addViewPortResizeBinding (
                [](void) {}
            );
        }
    }

    void SBImpl::destroyRenderer (void) {
        auto& collectionObj         = m_sandBoxInfo.resource.collectionObj;
        auto& defaultTexturePoolObj = m_sandBoxInfo.resource.defaultTexturePoolObj;

        {   /* Renderer */
            collectionObj->removeCollectionTypeInstance <Renderer::VKRenderer> ("DEFAULT");
        }
        {   /* Cmd buffers */
            collectionObj->removeCollectionTypeInstance <Renderer::VKCmdBuffer> ("DEFAULT_DRAW_OPS");
        }
        {   /* Cmd pool */
            collectionObj->removeCollectionTypeInstance <Renderer::VKCmdPool> ("DEFAULT_DRAW_OPS");
        }
        {   /* Semaphore */
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKSemaphore> (
                    "DEFAULT_RENDER_DONE_"   + std::to_string (i)
                );
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKSemaphore> (
                    "DEFAULT_IMG_AVAILABLE_" + std::to_string (i)
                );
        }
        {   /* Fence */
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKFence> (
                    "DEFAULT_IN_FLIGHT_" + std::to_string (i)
                );
        }
        {   /* Descriptor sets */
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorSet> ("SKY_BOX_ONE_TIME");
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorSet> ("DEFAULT_ONE_TIME");
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorSet> ("SKY_BOX_PER_FRAME");
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorSet> ("DEFAULT_PER_FRAME");
        }
        {   /* Descriptor pool */
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorPool> ("SKY_BOX");
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorPool> ("DEFAULT");
        }
        {   /* Pipeline */
            collectionObj->removeCollectionTypeInstance <Renderer::VKPipeline> ("SKY_BOX");
            collectionObj->removeCollectionTypeInstance <Renderer::VKPipeline> ("DEFAULT");
        }
        {   /* Frame buffer */
            auto swapChainObj = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain> ("DEFAULT");
            for (uint32_t i = 0; i < swapChainObj->getSwapChainImagesCount(); i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKFrameBuffer> (
                    "DEFAULT_" + std::to_string (i)
                );
        }
        {   /* Render pass */
            collectionObj->removeCollectionTypeInstance <Renderer::VKRenderPass> ("DEFAULT");
        }
        {   /* Sampler */
            collectionObj->removeCollectionTypeInstance <Renderer::VKSampler> ("SKY_BOX");
            collectionObj->removeCollectionTypeInstance <Renderer::VKSampler> ("DEFAULT");
        }
        {   /* Image */
            auto texturePool  = defaultTexturePoolObj->getTexturePool();
            auto swapChainObj = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain> ("DEFAULT");

            collectionObj->removeCollectionTypeInstance <Renderer::VKImage>     ("SKY_BOX_TEXTURE");
            for (auto const& [idx, info]: texturePool)
                collectionObj->removeCollectionTypeInstance <Renderer::VKImage> (
                    "DEFAULT_TEXTURE_"    + std::to_string (idx)
                );
            for (uint32_t i = 0; i < swapChainObj->getSwapChainImagesCount(); i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKImage> (
                    "DEFAULT_SWAP_CHAIN_" + std::to_string (i)
                );
            collectionObj->removeCollectionTypeInstance <Renderer::VKImage>     ("DEFAULT_DEPTH");
            collectionObj->removeCollectionTypeInstance <Renderer::VKImage>     ("DEFAULT_COLOR");
        }
        {   /* Buffer */
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer> (
                    "DEFAULT_LIGHT_INSTANCE_" + std::to_string (i)
                );
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer> (
                    "SKY_BOX_MESH_INSTANCE_"  + std::to_string (i)
                );
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer> (
                    "DEFAULT_MESH_INSTANCE_"  + std::to_string (i)
                );
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>     ("SKY_BOX_INDEX");
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>     ("DEFAULT_INDEX");
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>     ("SKY_BOX_VERTEX");
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>     ("DEFAULT_VERTEX");
        }
        {   /* Swap chain */
            collectionObj->removeCollectionTypeInstance <Renderer::VKSwapChain> ("DEFAULT");
        }
        {   /* Log device */
            collectionObj->removeCollectionTypeInstance <Renderer::VKLogDevice> ("DEFAULT");
        }
        {   /* Phy device */
            collectionObj->removeCollectionTypeInstance <Renderer::VKPhyDevice> ("DEFAULT");
        }
        {   /* Surface */
            collectionObj->removeCollectionTypeInstance <Renderer::VKSurface> ("DEFAULT");
        }
        {   /* Window */
            collectionObj->removeCollectionTypeInstance <Renderer::VKWindow> ("DEFAULT");
        }
        {   /* Instance */
            collectionObj->removeCollectionTypeInstance <Renderer::VKInstance> ("DEFAULT");
        }
        {   /* Log */
            collectionObj->removeCollectionTypeInstance <Log::LGImpl> ("DEFAULT");
        }
    }
}   // namespace SandBox