#include "../../Backend/Common.h"
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
#include "../SBTexturePool.h"
#include "../System/Batching/SYMeshBatching.h"
#include "../System/Batching/SYStdMeshInstanceBatching.h"
#include "../System/Batching/SYWireMeshInstanceBatching.h"
#include "../System/Batching/SYLightInstanceBatching.h"
#include "../SBImpl.h"
#include "../../Backend/Renderer/VKCmdList.h"
#include "../../Backend/Renderer/VKHelper.h"
#include "../SBComponentType.h"
#include "../SBRendererType.h"

namespace SandBox {
    /*  Naming convention
     *  +-------------------+-------------------+-------------------+-------------------+-------------------+
     *  |   [RENDER PASS]   |     [PIPELINE]    |       [DATA]      |     [OPTIONAL]    |       [IDX]       |
     *  +-------------------+-------------------+-------------------+-------------------+-------------------+
     *  |   N/A             |   N/A             |   N/A             |   CORE            |   N/A             |
     *  |                   |                   |                   |   SWAP_CHAIN      |   ?               |
     *  |                   |                   |                   |   COPY_OPS        |                   |
     *  |                   |                   |                   |   BLIT_OPS        |                   |
     *  |                   |                   |                   |   DRAW_OPS        |                   |
     *  |                   |                   |                   |   IN_FLIGHT       |                   |
     *  |                   |                   |                   |   IMG_AVAILABLE   |                   |
     *  |                   |                   |                   |   RENDER_DONE     |                   |
     *  +-------------------+-------------------+-------------------+-------------------+                   |
     *  |   S               |   N/A             |   N/A             |   N/A             |                   |
     *  |   G               |   DEFAULT         |   VERTEX          |   STAGING         |                   |
     *  |   F               |   CUBE            |   INDEX           |   PER_FRAME       |                   |
     *  |                   |   LIGHT           |   MESH_INSTANCE   |   OTHER           |                   |
     *  |                   |   WIRE            |   LIGHT_INSTANCE  |   CUBE            |                   |
     *  |                   |   SKY_BOX         |   NORMAL          |                   |                   |
     *  |                   |   DEBUG           |   POSITION        |                   |                   |
     *  |                   |                   |   COLOR           |                   |                   |
     *  |                   |                   |   DEPTH           |                   |                   |
     *  |                   |                   |   GBUFFER         |                   |                   |
     *  |                   |                   |   TEXTURE         |                   |                   |
     *  +-------------------+-------------------+-------------------+-------------------+-------------------+
    */
    void SBImpl::configRendererCore (void) {
        auto& collectionObj = m_sandBoxInfo.resource.collectionObj;

        {   /* Register collection types */
            collectionObj->registerCollectionType <Log::LGImpl>();
            collectionObj->registerCollectionType <Renderer::VKInstance>();
            collectionObj->registerCollectionType <Renderer::VKWindow>();
            collectionObj->registerCollectionType <Renderer::VKSurface>();
            collectionObj->registerCollectionType <Renderer::VKPhyDevice>();
            collectionObj->registerCollectionType <Renderer::VKLogDevice>();
            collectionObj->registerCollectionType <Renderer::VKSwapChain>();
            collectionObj->registerCollectionType <Renderer::VKBuffer>();
            collectionObj->registerCollectionType <Renderer::VKImage>();
            collectionObj->registerCollectionType <Renderer::VKSampler>();
            collectionObj->registerCollectionType <Renderer::VKRenderPass>();
            collectionObj->registerCollectionType <Renderer::VKFrameBuffer>();
            collectionObj->registerCollectionType <Renderer::VKPipeline>();
            collectionObj->registerCollectionType <Renderer::VKDescriptorPool>();
            collectionObj->registerCollectionType <Renderer::VKDescriptorSet>();
            collectionObj->registerCollectionType <Renderer::VKFence>();
            collectionObj->registerCollectionType <Renderer::VKSemaphore>();
            collectionObj->registerCollectionType <Renderer::VKCmdPool>();
            collectionObj->registerCollectionType <Renderer::VKCmdBuffer>();
            collectionObj->registerCollectionType <Renderer::VKRenderer>();
        }
        {   /* Log              [CORE] */
            auto logObj = new Log::LGImpl();
            logObj->initLogInfo     ("Build/Log/Renderer",   __FILE__);
            logObj->updateLogConfig (Log::LOG_LEVEL_INFO,    Log::LOG_SINK_FILE);
            logObj->updateLogConfig (Log::LOG_LEVEL_WARNING, Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);
            logObj->updateLogConfig (Log::LOG_LEVEL_ERROR,   Log::LOG_SINK_CONSOLE | Log::LOG_SINK_FILE);

            collectionObj->addCollectionTypeInstance <Log::LGImpl> ("CORE", logObj);
        }
        {   /* Instance         [CORE] */
            auto logObj      = collectionObj->getCollectionTypeInstance <Log::LGImpl> ("CORE");
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

            collectionObj->addCollectionTypeInstance <Renderer::VKInstance> ("CORE", instanceObj);
        }
        {   /* Window           [CORE] */
            auto logObj    = collectionObj->getCollectionTypeInstance <Log::LGImpl> ("CORE");
            auto windowObj = new Renderer::VKWindow (logObj);
            windowObj->initWindowInfo (
                1200,
                900,
                "EngineV2"
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKWindow> ("CORE", windowObj);
            windowObj->toggleKeyEventCallback (
                true
            );
            /* Set key event bindings */
            windowObj->setKeyEventBinding (GLFW_KEY_ESCAPE,
                [windowObj](void) {
                    windowObj->toggleWindowClosed (true);
                }
            );
        }
        {   /* Surface          [CORE] */
            auto logObj      = collectionObj->getCollectionTypeInstance <Log::LGImpl>          ("CORE");
            auto instanceObj = collectionObj->getCollectionTypeInstance <Renderer::VKInstance> ("CORE");
            auto windowObj   = collectionObj->getCollectionTypeInstance <Renderer::VKWindow>   ("CORE");
            auto surfaceObj  = new Renderer::VKSurface (logObj, instanceObj, windowObj);
            surfaceObj->initSurfaceInfo();

            collectionObj->addCollectionTypeInstance <Renderer::VKSurface> ("CORE", surfaceObj);
        }
        {   /* Phy device       [CORE] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>          ("CORE");
            auto instanceObj  = collectionObj->getCollectionTypeInstance <Renderer::VKInstance> ("CORE");
            auto surfaceObj   = collectionObj->getCollectionTypeInstance <Renderer::VKSurface>  ("CORE");
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

            collectionObj->addCollectionTypeInstance <Renderer::VKPhyDevice> ("CORE", phyDeviceObj);
        }
        {   /* Log device       [CORE] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("CORE");
            auto instanceObj  = collectionObj->getCollectionTypeInstance <Renderer::VKInstance>  ("CORE");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("CORE");
            auto logDeviceObj = new Renderer::VKLogDevice (logObj, instanceObj, phyDeviceObj);
            logDeviceObj->initLogDeviceInfo();

            collectionObj->addCollectionTypeInstance <Renderer::VKLogDevice> ("CORE", logDeviceObj);
        }
        {   /* Swap chain       [CORE] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("CORE");
            auto windowObj    = collectionObj->getCollectionTypeInstance <Renderer::VKWindow>    ("CORE");
            auto surfaceObj   = collectionObj->getCollectionTypeInstance <Renderer::VKSurface>   ("CORE");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("CORE");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("CORE");
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

            collectionObj->addCollectionTypeInstance <Renderer::VKSwapChain> ("CORE", swapChainObj);
        }

        {   /* Image            [SWAP_CHAIN_?] */
            auto logObj       = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("CORE");
            auto phyDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("CORE");
            auto logDeviceObj = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("CORE");
            auto swapChainObj = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain> ("CORE");
            auto images       = swapChainObj->getSwapChainImages();
            size_t loopIdx    = 0;

            for (auto const& image: images) {
                auto imageObj = new Renderer::VKImage (logObj, phyDeviceObj, logDeviceObj);
                imageObj->initImageInfo (
                    1,
                    0,
                    1,
                    swapChainObj->getSwapChainFormat(),
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_VIEW_TYPE_2D,
                    image
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKImage> (
                    "SWAP_CHAIN_" + std::to_string (loopIdx++),
                    imageObj
                );
            }
        }
    }

    void SBImpl::configRendererSPass (void) {
        auto& shadowImageWidth  = m_sandBoxInfo.meta.shadowImageWidth;
        auto& shadowImageHeight = m_sandBoxInfo.meta.shadowImageHeight;
        auto& sceneObj          = m_sandBoxInfo.resource.sceneObj;
        auto& collectionObj     = m_sandBoxInfo.resource.collectionObj;

        auto logObj             = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("CORE");
        auto phyDeviceObj       = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("CORE");
        auto logDeviceObj       = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("CORE");

        {   /* Buffer           [S_DEFAULT_VERTEX_STAGING] */
            auto batchingObj = sceneObj->getSystem <SYMeshBatching>();
            auto vertices    = batchingObj->getBatchedVertices (TAG_TYPE_STD);
            auto bufferObj   = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
            bufferObj->initBufferInfo (
                vertices.size() * sizeof (Vertex),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                {
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                false
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("S_DEFAULT_VERTEX_STAGING", bufferObj);
            bufferObj->updateBuffer (
                vertices.data(),
                true
            );
        }
        {   /* Buffer           [S_DEFAULT_INDEX_STAGING] */
            auto batchingObj = sceneObj->getSystem <SYMeshBatching>();
            auto indices     = batchingObj->getBatchedIndices (TAG_TYPE_STD);
            auto bufferObj   = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
            bufferObj->initBufferInfo (
                indices.size() * sizeof (IndexType),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                {
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                false
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("S_DEFAULT_INDEX_STAGING", bufferObj);
            bufferObj->updateBuffer (
                indices.data(),
                true
            );
        }
        {   /* Buffer           [S_DEFAULT_VERTEX] */
            auto batchingObj = sceneObj->getSystem <SYMeshBatching>();
            auto vertices    = batchingObj->getBatchedVertices (TAG_TYPE_STD);
            auto bufferObj   = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
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

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("S_DEFAULT_VERTEX", bufferObj);
        }
        {   /* Buffer           [S_DEFAULT_INDEX] */
            auto batchingObj = sceneObj->getSystem <SYMeshBatching>();
            auto indices     = batchingObj->getBatchedIndices (TAG_TYPE_STD);
            auto bufferObj   = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
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

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("S_DEFAULT_INDEX", bufferObj);
        }
        {   /* Buffer           [S_DEFAULT_MESH_INSTANCE_?] */
            auto batchingObj = sceneObj->getSystem <SYStdMeshInstanceBatching>();
            auto instances   = batchingObj->getBatchedMeshInstancesLite (TAG_TYPE_STD);
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
                    "S_DEFAULT_MESH_INSTANCE_" + std::to_string (i),
                    bufferObj
                );
            }
        }

        {   /* Image            [S_DEFAULT_DEPTH_?] */
            auto batchingObj          = sceneObj->getSystem <SYLightInstanceBatching>();
            auto lightTypeOffsets     = batchingObj->getLightTypeOffsets();
            uint32_t otherLightsCount = lightTypeOffsets->pointLightsOffset;

            for (uint32_t i = 0; i < otherLightsCount; i++) {
                auto imageObj    = new Renderer::VKImage (logObj, phyDeviceObj, logDeviceObj);
                auto imageFormat = Renderer::getSupportedFormat (
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
                    shadowImageWidth,
                    shadowImageHeight,
                    1,
                    0,
                    1,
                    0,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    imageFormat,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    {
                        phyDeviceObj->getGraphicsQueueFamilyIdx()
                    },
                    VK_IMAGE_ASPECT_DEPTH_BIT,
                    VK_IMAGE_VIEW_TYPE_2D
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKImage> (
                    "S_DEFAULT_DEPTH_" + std::to_string (i),
                    imageObj
                );
            }
        }
        {   /* Image            [F_LIGHT_DEPTH_CUBE_?] */
            auto batchingObj          = sceneObj->getSystem <SYLightInstanceBatching>();
            auto lightTypeOffsets     = batchingObj->getLightTypeOffsets();
            uint32_t pointLightsCount = lightTypeOffsets->lightsCount - lightTypeOffsets->pointLightsOffset;

            for (uint32_t i = 0; i < pointLightsCount; i++) {
                auto imageObj    = new Renderer::VKImage (logObj, phyDeviceObj, logDeviceObj);
                auto imageFormat = Renderer::getSupportedFormat (
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
                    shadowImageWidth,
                    shadowImageHeight,
                    1,
                    0,
                    6,
                    VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    imageFormat,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_IMAGE_TILING_OPTIMAL,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    {
                        phyDeviceObj->getGraphicsQueueFamilyIdx()
                    },
                    VK_IMAGE_ASPECT_DEPTH_BIT,
                    VK_IMAGE_VIEW_TYPE_CUBE
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKImage> (
                    "F_LIGHT_DEPTH_CUBE_" + std::to_string (i),
                    imageObj
                );
            }
        }
        {   /* Image            [S_CUBE_DEPTH_?] */
            auto batchingObj          = sceneObj->getSystem <SYLightInstanceBatching>();
            auto lightTypeOffsets     = batchingObj->getLightTypeOffsets();
            uint32_t pointLightsCount = lightTypeOffsets->lightsCount - lightTypeOffsets->pointLightsOffset;

            for (uint32_t i = 0; i < pointLightsCount; i++) {
                auto depthImageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage> (
                    "F_LIGHT_DEPTH_CUBE_" + std::to_string (i)
                );
                for (uint32_t cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx++) {
                    auto imageObj  = new Renderer::VKImage (logObj, phyDeviceObj, logDeviceObj);
                    imageObj->initImageInfo (
                        depthImageObj->getImageMipLevels(),
                        cubeFaceIdx,
                        1,
                        depthImageObj->getImageFormat(),
                        depthImageObj->getImageAspectFlags(),
                        VK_IMAGE_VIEW_TYPE_2D,
                        *depthImageObj->getImage()
                    );

                    collectionObj->addCollectionTypeInstance <Renderer::VKImage> (
                        "S_CUBE_DEPTH_" + std::to_string ((i * 6) + cubeFaceIdx),
                        imageObj
                    );
                }
            }
        }

        {   /* Render pass      [S] */
            auto imageObj      = collectionObj->getCollectionTypeInstance <Renderer::VKImage> ("S_DEFAULT_DEPTH_0");
            auto renderPassObj = new Renderer::VKRenderPass (logObj, logDeviceObj);
            renderPassObj->initRenderPassInfo();

            /* Depth attachment */
            renderPassObj->addRenderPassAttachment (
                0,
                imageObj->getImageFormat(),
                imageObj->getImageSamplesCount(),
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );
            /* Sub pass */
            /* The first transition for an attached image of a render pass will be from the initial layout for the render
             * pass to the reference layout for the first sub pass that uses the image. And, the last transition for an
             * attached image will be from reference layout of the final sub pass that uses the attachment to the final
             * layout for the render pass
            */
            auto inputAttachmentReferences   = std::vector <VkAttachmentReference> {};
            auto colorAttachmentReferences   = std::vector <VkAttachmentReference> {};
            auto depthAttachmentReference    =
                renderPassObj->createAttachmentReference (
                    0,
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                );
            auto resolveAttachmentReferences = std::vector <VkAttachmentReference> {};
            renderPassObj->addSubPass (
                inputAttachmentReferences,
                colorAttachmentReferences,
                &depthAttachmentReference,
                resolveAttachmentReferences
            );
            /* Dependency */
            /* Read as, "In sub pass [dstSubPassIdx], at this pipeline stage [dstStageMask] wait before performing these
             * operations [dstAccessMask], until all operations of this type [srcAccessMask] at these pipeline stages
             * [srcStageMask] occuring in submission order prior to vkCmdBeginRenderPass [VK_SUBPASS_EXTERNAL] have
             * completed execution"
            */
            renderPassObj->addSubPassDependency (
                0,
                VK_SUBPASS_EXTERNAL,
                0,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                VK_ACCESS_SHADER_READ_BIT,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKRenderPass> ("S", renderPassObj);
        }
        {   /* Frame buffer     [S_?] */
            auto batchingObj          = sceneObj->getSystem <SYLightInstanceBatching>();
            auto lightTypeOffsets     = batchingObj->getLightTypeOffsets();
            uint32_t otherLightsCount = lightTypeOffsets->pointLightsOffset;
            uint32_t pointLightsCount = lightTypeOffsets->lightsCount - lightTypeOffsets->pointLightsOffset;
            size_t activeLightIdx     = 0;

            auto renderPassObj        = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass> ("S");

            for (uint32_t i = 0; i < otherLightsCount; i++) {
                auto imageObj  = collectionObj->getCollectionTypeInstance <Renderer::VKImage> (
                    "S_DEFAULT_DEPTH_" + std::to_string (i)
                );
                auto bufferObj = new Renderer::VKFrameBuffer (logObj, logDeviceObj, renderPassObj);
                bufferObj->initFrameBufferInfo (
                    imageObj->getImageExtent().width,
                    imageObj->getImageExtent().height,
                    imageObj->getImageLayersCount(),
                    {
                        *imageObj->getImageView()               /* Attachment idx 0 */
                    }
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKFrameBuffer> (
                    "S_" + std::to_string (activeLightIdx++),
                    bufferObj
                );
            }
            for (uint32_t i = 0; i < pointLightsCount; i++) {
                for (uint32_t cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx++) {
                    auto imageObj  = collectionObj->getCollectionTypeInstance <Renderer::VKImage> (
                        "S_CUBE_DEPTH_" + std::to_string ((i * 6) + cubeFaceIdx)
                    );
                    auto bufferObj = new Renderer::VKFrameBuffer (logObj, logDeviceObj, renderPassObj);
                    bufferObj->initFrameBufferInfo (
                        /* Note that, we cannot use the helper method to get the image extent here, because the image
                         * object was created using the `image view only` constructor
                        */
                        shadowImageWidth,
                        shadowImageHeight,
                        imageObj->getImageLayersCount(),
                        {
                            *imageObj->getImageView()           /* Attachment idx 0 */
                        }
                    );

                    collectionObj->addCollectionTypeInstance <Renderer::VKFrameBuffer> (
                        "S_" + std::to_string (activeLightIdx++),
                        bufferObj
                    );
                }
            }
        }

        {   /* Pipeline         [S_DEFAULT] */
            auto renderPassObj = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass> ("S");
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

            auto vertexBindingDescriptions   = std::vector <VkVertexInputBindingDescription> {};
            auto vertexAttributeDescriptions = std::vector <VkVertexInputAttributeDescription> {};
            auto colorBlendAttachments       = std::vector <VkPipelineColorBlendAttachmentState> {};
            auto colorBlendConstants         = std::vector <float> {};
            auto dynamicStates               = std::vector <VkDynamicState> {};
            auto viewPorts                   = std::vector <VkViewport> {};
            auto scissors                    = std::vector <VkRect2D> {};
            auto perFrameBindingFlags        = std::vector <VkDescriptorBindingFlags> {};
            auto perFrameLayoutBindings      = std::vector <VkDescriptorSetLayoutBinding> {};

            {   /* Vertex input */
                vertexBindingDescriptions   = {
                    pipelineObj->createVertexBindingDescription (
                        0,
                        sizeof (Vertex),
                        VK_VERTEX_INPUT_RATE_VERTEX
                    )
                };
                vertexAttributeDescriptions = {
                    pipelineObj->createVertexAttributeDescription (
                        0,
                        0,
                        offsetof (Vertex, meta.position),
                        VK_FORMAT_R32G32B32_SFLOAT
                    )
                };
                pipelineObj->createVertexInputState (
                    vertexBindingDescriptions,
                    vertexAttributeDescriptions
                );
            }
            {   /* Input assembly */
                pipelineObj->createInputAssemblyState (
                    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                    VK_FALSE
                );
            }
            {   /* Shader stage */
                pipelineObj->addShaderStage (
                    VK_SHADER_STAGE_VERTEX_BIT,
                    "Build/Bin/Shadow[VERT].spv",
                    "main"
                );
            }
            {   /* Depth stencil */
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
            }
            {   /* Rasterization */
                /* Because the shadow image is limited by resolution, one texel in our shadow image will store a depth
                 * value sampled by multiple fragments which may have slightly different depths in reality. For example,
                 * the depth of a fragment may be slightly larger than the depth stored, making it incorrectly show up
                 * as in shadow resulting in a Moiré-like pattern called shadow acne (Floating point precision errors
                 * when we compare fragment depth with sampled depth can also be a cause for shadow acne)
                 *                                      +  /                   /+               +
                 *                                      |/                   /  |               | <== 1x Texel
                 *                                     /|                  /    |               |
                 *     -------->                  +--/--+                /+-----+               +
                 *   O -------->                  |/                   /  |
                 *     -------->                 /|                  /    |                     / <-- 1x Fragment
                 *                          +--/--+                /+-----+
                 *                          |/                   /  |
                 *                         /|                  /    |
                 *                    +--/--+                /+-----+
                 *                     /                   /
                 *                    Before               After
                 *                    depth bias           depth bias
                 *
                 * We can solve this issue with a small little hack called a shadow bias where we simply offset the depth
                 * of the surface or the shadow image by a small bias amount such that the fragments are not incorrectly
                 * considered shadowed
                 *
                 * Note that, choosing the correct bias values requires some tweaking as this will be different for each
                 * scene, but most of the time it's simply a matter of slowly incrementing the bias until all acne is
                 * removed
                 *
                 * A disadvantage of using a shadow bias is that you're applying an offset to the actual depth of objects.
                 * As a result, the bias may become large enough to see a visible offset of shadows compared to the actual
                 * object locations. This shadow artifact is called peter panning since objects seem slightly detached
                 * from their shadows. We can solve this by culling all front faces when rendering the shadow image. This
                 * helps to remove shadow acne on the front faces, thus allowing us to reduce the bias and effectively
                 * minimizing peter panning. But this only works for solid objects that actually have an inside without
                 * openings
                */
                pipelineObj->createRasterizationState (
                    1.0f,
                    VK_POLYGON_MODE_FILL,
                    VK_CULL_MODE_BACK_BIT,
                    VK_FRONT_FACE_COUNTER_CLOCKWISE,
                    VK_TRUE,
                    0.0f,       /* Set via dynamic state */
                    0.0f        /* Set via dynamic state */
                );
            }
            {   /* Multi sample */
                pipelineObj->createMultiSampleState (
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_FALSE,
                    0.0f
                );
            }
            {   /* Color blend */
                colorBlendAttachments = {
                    pipelineObj->createColorBlendAttachment (
                        VK_FALSE,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
                    )
                };
                colorBlendConstants   = {
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
            }
            {   /* Dynamic state */
                dynamicStates = {
                    VK_DYNAMIC_STATE_VIEWPORT,
                    VK_DYNAMIC_STATE_SCISSOR,
                    VK_DYNAMIC_STATE_DEPTH_BIAS
                };
                pipelineObj->createDynamicState (dynamicStates);
            }
            {   /* View port */
                viewPorts = {};
                scissors  = {};
                pipelineObj->createViewPortState (
                    viewPorts,
                    scissors,
                    1,
                    1
                );
            }
            {   /* Descriptor set layout */
                perFrameBindingFlags   = {
                    0
                };
                perFrameLayoutBindings = {
                    pipelineObj->createDescriptorSetLayoutBinding (
                        0,
                        1,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        VK_SHADER_STAGE_VERTEX_BIT
                    )
                };
                /* Layout 0 */
                pipelineObj->addDescriptorSetLayout (
                    0,
                    perFrameBindingFlags,
                    perFrameLayoutBindings
                );
            }
            {   /* Push constant range */
                pipelineObj->addPushConstantRange (
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    sizeof (ActiveLightPC)
                );
            }

            collectionObj->addCollectionTypeInstance <Renderer::VKPipeline> ("S_DEFAULT", pipelineObj);
            pipelineObj->destroyShaderModules();
        }
        {   /* Descriptor pool  [S_DEFAULT] */
            auto descPoolObj = new Renderer::VKDescriptorPool (logObj, logDeviceObj);
            descPoolObj->initDescriptorPoolInfo (
                0,
                g_maxFramesInFlight
            );
            descPoolObj->addDescriptorPoolSize (
                g_maxFramesInFlight,                            /* S_DEFAULT_MESH_INSTANCE_? */
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorPool> ("S_DEFAULT", descPoolObj);
        }
        {   /* Descriptor sets  [S_DEFAULT_PER_FRAME] */
            auto pipelineObj = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>       ("S_DEFAULT");
            auto descPoolObj = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorPool> ("S_DEFAULT");
            auto descSetObj  = new Renderer::VKDescriptorSet (logObj, logDeviceObj, descPoolObj);
            descSetObj->initDescriptorSetInfo (
                g_maxFramesInFlight,
                pipelineObj->getDescriptorSetLayouts()[0]
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorSet> ("S_DEFAULT_PER_FRAME", descSetObj);

            typedef std::vector        <std::vector <VkDescriptorBufferInfo>> descriptorBufferInfosPool;
            auto descriptorImageInfos = std::vector <VkDescriptorImageInfo> {};
            std::unordered_map <uint32_t, descriptorBufferInfosPool> bindingToBufferInfosMap;

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                {   /* Binding 0 */
                    auto bufferObj             = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                        "S_DEFAULT_MESH_INSTANCE_" + std::to_string (i)
                    );
                    auto descriptorBufferInfos = std::vector <VkDescriptorBufferInfo> {
                        descSetObj->createDescriptorBufferInfo (
                            *bufferObj->getBuffer(),
                            0,
                            bufferObj->getBufferSize()
                        )
                    };
                    bindingToBufferInfosMap[0].push_back (
                        descriptorBufferInfos
                    );
                }
            }
            /* Write and update descriptor sets */
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                {   /* Binding 0 */
                    descSetObj->addWriteDescriptorSet (
                        0,
                        static_cast <uint32_t> (bindingToBufferInfosMap[0][i].size()),
                        0,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        descSetObj->getDescriptorSets()[i],
                        bindingToBufferInfosMap[0][i],
                        descriptorImageInfos
                    );
                }
            }
            descSetObj->updateDescriptorSets();
        }

        {   /* Pipeline         [S_CUBE] */
            auto renderPassObj   = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass> ("S");
            auto basePipelineObj = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>   ("S_DEFAULT");
            auto pipelineObj     = new Renderer::VKPipeline (logObj, logDeviceObj, renderPassObj);
            pipelineObj->initPipelineInfo (
                 VK_PIPELINE_CREATE_DERIVATIVE_BIT,
                 0,
                -1,
                *basePipelineObj->getPipeline()
            );

            auto vertexBindingDescriptions   = std::vector <VkVertexInputBindingDescription> {};
            auto vertexAttributeDescriptions = std::vector <VkVertexInputAttributeDescription> {};
            auto colorBlendAttachments       = std::vector <VkPipelineColorBlendAttachmentState> {};
            auto colorBlendConstants         = std::vector <float> {};
            auto dynamicStates               = std::vector <VkDynamicState> {};
            auto viewPorts                   = std::vector <VkViewport> {};
            auto scissors                    = std::vector <VkRect2D> {};
            auto perFrameBindingFlags        = std::vector <VkDescriptorBindingFlags> {};
            auto perFrameLayoutBindings      = std::vector <VkDescriptorSetLayoutBinding> {};

            {   /* Vertex input */
                vertexBindingDescriptions   = {
                    pipelineObj->createVertexBindingDescription (
                        0,
                        sizeof (Vertex),
                        VK_VERTEX_INPUT_RATE_VERTEX
                    )
                };
                vertexAttributeDescriptions = {
                    pipelineObj->createVertexAttributeDescription (
                        0,
                        0,
                        offsetof (Vertex, meta.position),
                        VK_FORMAT_R32G32B32_SFLOAT
                    )
                };
                pipelineObj->createVertexInputState (
                    vertexBindingDescriptions,
                    vertexAttributeDescriptions
                );
            }
            {   /* Input assembly */
                pipelineObj->createInputAssemblyState (
                    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                    VK_FALSE
                );
            }
            {   /* Shader stage */
                pipelineObj->addShaderStage (
                    VK_SHADER_STAGE_VERTEX_BIT,
                    "Build/Bin/ShadowCube[VERT].spv",
                    "main"
                );
                pipelineObj->addShaderStage (
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                    "Build/Bin/ShadowCube[FRAG].spv",
                    "main"
                );
            }
            {   /* Depth stencil */
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
            }
            {   /* Rasterization */
                pipelineObj->createRasterizationState (
                    1.0f,
                    VK_POLYGON_MODE_FILL,
                    VK_CULL_MODE_FRONT_BIT,         /* Compensate for flip in winding order */
                    VK_FRONT_FACE_COUNTER_CLOCKWISE,
                    VK_FALSE,
                    0.0f,
                    0.0f
                );
            }
            {   /* Multi sample */
                pipelineObj->createMultiSampleState (
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_FALSE,
                    0.0f
                );
            }
            {   /* Color blend */
                colorBlendAttachments = {
                    pipelineObj->createColorBlendAttachment (
                        VK_FALSE,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
                    )
                };
                colorBlendConstants   = {
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
            }
            {   /* Dynamic state */
                dynamicStates = {
                    VK_DYNAMIC_STATE_VIEWPORT,
                    VK_DYNAMIC_STATE_SCISSOR
                };
                pipelineObj->createDynamicState (dynamicStates);
            }
            {   /* View port */
                viewPorts = {};
                scissors  = {};
                pipelineObj->createViewPortState (
                    viewPorts,
                    scissors,
                    1,
                    1
                );
            }
            {   /* Descriptor set layout */
                perFrameBindingFlags   = {
                    0
                };
                perFrameLayoutBindings = {
                    pipelineObj->createDescriptorSetLayoutBinding (
                        0,
                        1,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        VK_SHADER_STAGE_VERTEX_BIT
                    )
                };
                /* Layout 0 */
                pipelineObj->addDescriptorSetLayout (
                    0,
                    perFrameBindingFlags,
                    perFrameLayoutBindings
                );
            }
            {   /* Push constant range */
                pipelineObj->addPushConstantRange (
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    sizeof (ActiveLightPC)
                );
            }

            collectionObj->addCollectionTypeInstance <Renderer::VKPipeline> ("S_CUBE", pipelineObj);
            pipelineObj->destroyShaderModules();
        }
        {   /* Descriptor pool  [S_CUBE] */
            auto descPoolObj = new Renderer::VKDescriptorPool (logObj, logDeviceObj);
            descPoolObj->initDescriptorPoolInfo (
                0,
                g_maxFramesInFlight
            );
            descPoolObj->addDescriptorPoolSize (
                g_maxFramesInFlight,                            /* S_DEFAULT_MESH_INSTANCE_? */
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorPool> ("S_CUBE", descPoolObj);
        }
        {   /* Descriptor sets  [S_CUBE_PER_FRAME] */
            auto pipelineObj = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>       ("S_CUBE");
            auto descPoolObj = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorPool> ("S_CUBE");
            auto descSetObj  = new Renderer::VKDescriptorSet (logObj, logDeviceObj, descPoolObj);
            descSetObj->initDescriptorSetInfo (
                g_maxFramesInFlight,
                pipelineObj->getDescriptorSetLayouts()[0]
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorSet> ("S_CUBE_PER_FRAME", descSetObj);

            typedef std::vector        <std::vector <VkDescriptorBufferInfo>> descriptorBufferInfosPool;
            auto descriptorImageInfos = std::vector <VkDescriptorImageInfo> {};
            std::unordered_map <uint32_t, descriptorBufferInfosPool> bindingToBufferInfosMap;

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                {   /* Binding 0 */
                    auto bufferObj             = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                        "S_DEFAULT_MESH_INSTANCE_" + std::to_string (i)
                    );
                    auto descriptorBufferInfos = std::vector <VkDescriptorBufferInfo> {
                        descSetObj->createDescriptorBufferInfo (
                            *bufferObj->getBuffer(),
                            0,
                            bufferObj->getBufferSize()
                        )
                    };
                    bindingToBufferInfosMap[0].push_back (
                        descriptorBufferInfos
                    );
                }
            }
            /* Write and update descriptor sets */
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                {   /* Binding 0 */
                    descSetObj->addWriteDescriptorSet (
                        0,
                        static_cast <uint32_t> (bindingToBufferInfosMap[0][i].size()),
                        0,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        descSetObj->getDescriptorSets()[i],
                        bindingToBufferInfosMap[0][i],
                        descriptorImageInfos
                    );
                }
            }
            descSetObj->updateDescriptorSets();
        }
    }

    void SBImpl::configRendererGPass (void) {
        auto& sceneObj          = m_sandBoxInfo.resource.sceneObj;
        auto& collectionObj     = m_sandBoxInfo.resource.collectionObj;
        auto& stdTexturePoolObj = m_sandBoxInfo.resource.stdTexturePoolObj;

        auto logObj             = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("CORE");
        auto phyDeviceObj       = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("CORE");
        auto logDeviceObj       = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("CORE");
        auto swapChainObj       = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain> ("CORE");
        auto stdTexturePool     = stdTexturePoolObj->getTexturePool();

        {   /* Buffer           [G_DEFAULT_MESH_INSTANCE_?] */
            auto batchingObj = sceneObj->getSystem <SYStdMeshInstanceBatching>();
            auto instances   = batchingObj->getBatchedMeshInstances (TAG_TYPE_STD);

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
                    "G_DEFAULT_MESH_INSTANCE_" + std::to_string (i),
                    bufferObj
                );
            }
        }
        {   /* Buffer           [G_DEFAULT_TEXTURE_STAGING_?] */
            for (auto const& [idx, info]: stdTexturePool) {
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
                    "G_DEFAULT_TEXTURE_STAGING_" + std::to_string (idx),
                    bufferObj
                );
                bufferObj->updateBuffer (
                    info.resource.data,
                    true
                );
                stdTexturePoolObj->destroyImage (idx);
            }
        }

        {   /* Image            [G_DEFAULT_NORMAL] */
            auto imageObj = new Renderer::VKImage (logObj, phyDeviceObj, logDeviceObj);
            imageObj->initImageInfo (
                swapChainObj->getSwapChainExtent()->width,
                swapChainObj->getSwapChainExtent()->height,
                1,
                0,
                1,
                0,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_FORMAT_R16G16B16A16_SFLOAT,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_TILING_OPTIMAL,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                {
                    phyDeviceObj->getGraphicsQueueFamilyIdx()
                },
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_VIEW_TYPE_2D
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKImage> ("G_DEFAULT_NORMAL", imageObj);
        }
        {   /* Image            [G_DEFAULT_POSITION] */
            auto imageObj = new Renderer::VKImage (logObj, phyDeviceObj, logDeviceObj);
            imageObj->initImageInfo (
                swapChainObj->getSwapChainExtent()->width,
                swapChainObj->getSwapChainExtent()->height,
                1,
                0,
                1,
                0,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_FORMAT_R16G16B16A16_SFLOAT,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_TILING_OPTIMAL,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                {
                    phyDeviceObj->getGraphicsQueueFamilyIdx()
                },
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_VIEW_TYPE_2D
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKImage> ("G_DEFAULT_POSITION", imageObj);
        }
        {   /* Image            [G_DEFAULT_COLOR_0] */
            auto imageObj = new Renderer::VKImage (logObj, phyDeviceObj, logDeviceObj);
            imageObj->initImageInfo (
                swapChainObj->getSwapChainExtent()->width,
                swapChainObj->getSwapChainExtent()->height,
                1,
                0,
                1,
                0,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_TILING_OPTIMAL,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                {
                    phyDeviceObj->getGraphicsQueueFamilyIdx()
                },
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_VIEW_TYPE_2D
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKImage> ("G_DEFAULT_COLOR_0", imageObj);
        }
        {   /* Image            [G_DEFAULT_COLOR_1] */
            auto imageObj = new Renderer::VKImage (logObj, phyDeviceObj, logDeviceObj);
            imageObj->initImageInfo (
                swapChainObj->getSwapChainExtent()->width,
                swapChainObj->getSwapChainExtent()->height,
                1,
                0,
                1,
                0,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_TILING_OPTIMAL,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                {
                    phyDeviceObj->getGraphicsQueueFamilyIdx()
                },
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_VIEW_TYPE_2D
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKImage> ("G_DEFAULT_COLOR_1", imageObj);
        }
        {   /* Image            [G_DEFAULT_COLOR_2] */
            auto imageObj = new Renderer::VKImage (logObj, phyDeviceObj, logDeviceObj);
            imageObj->initImageInfo (
                swapChainObj->getSwapChainExtent()->width,
                swapChainObj->getSwapChainExtent()->height,
                1,
                0,
                1,
                0,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_TILING_OPTIMAL,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                {
                    phyDeviceObj->getGraphicsQueueFamilyIdx()
                },
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_VIEW_TYPE_2D
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKImage> ("G_DEFAULT_COLOR_2", imageObj);
        }
        {   /* Image            [G_DEFAULT_DEPTH] */
            auto imageObj    = new Renderer::VKImage (logObj, phyDeviceObj, logDeviceObj);
            auto imageFormat = Renderer::getSupportedFormat (
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
                0,
                1,
                0,
                VK_IMAGE_LAYOUT_UNDEFINED,
                imageFormat,
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_TILING_OPTIMAL,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                {
                    phyDeviceObj->getGraphicsQueueFamilyIdx()
                },
                VK_IMAGE_ASPECT_DEPTH_BIT,
                VK_IMAGE_VIEW_TYPE_2D
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKImage> ("G_DEFAULT_DEPTH", imageObj);
        }
        {   /* Image            [G_DEFAULT_TEXTURE_?] */
            for (auto const& [idx, info]: stdTexturePool) {
                auto imageObj = new Renderer::VKImage (logObj, phyDeviceObj, logDeviceObj);
                /* To calculate the number of levels in the mip chain, we use the max function to select the largest
                 * dimension. The log2 function calculates how many times that dimension can be divided by 2. The floor
                 * function handles cases where the largest dimension is not a power of 2. Finally, 1 is added so that
                 * the original texture has a mip level
                */
                uint32_t mipLevels = static_cast <uint32_t> (
                    std::floor (std::log2 (std::max (
                        info.meta.width,
                        info.meta.height
                    )))
                ) + 1;
                imageObj->initImageInfo (
                    static_cast <uint32_t> (info.meta.width),
                    static_cast <uint32_t> (info.meta.height),
                    mipLevels,
                    0,
                    1,
                    0,
                    VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_FORMAT_R8G8B8A8_SRGB,
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
                    "G_DEFAULT_TEXTURE_" + std::to_string (idx),
                    imageObj
                );
            }
        }
        {   /* Sampler          [G_DEFAULT_TEXTURE] */
            auto samplerObj = new Renderer::VKSampler (logObj, logDeviceObj);
            samplerObj->initSamplerInfo (
                0.0f,
                0.0f,
                VK_LOD_CLAMP_NONE,
                0.0f,
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VK_SAMPLER_MIPMAP_MODE_LINEAR,
                VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                VK_TRUE
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKSampler> ("G_DEFAULT_TEXTURE", samplerObj);
        }

        auto normalImageObj   = collectionObj->getCollectionTypeInstance <Renderer::VKImage> ("G_DEFAULT_NORMAL");
        auto positionImageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage> ("G_DEFAULT_POSITION");
        auto color0ImageObj   = collectionObj->getCollectionTypeInstance <Renderer::VKImage> ("G_DEFAULT_COLOR_0");
        auto color1ImageObj   = collectionObj->getCollectionTypeInstance <Renderer::VKImage> ("G_DEFAULT_COLOR_1");
        auto color2ImageObj   = collectionObj->getCollectionTypeInstance <Renderer::VKImage> ("G_DEFAULT_COLOR_2");
        auto depthImageObj    = collectionObj->getCollectionTypeInstance <Renderer::VKImage> ("G_DEFAULT_DEPTH");

        {   /* Render pass      [G] */
            auto renderPassObj = new Renderer::VKRenderPass (logObj, logDeviceObj);
            renderPassObj->initRenderPassInfo();

            /* Normal attachment */
            renderPassObj->addRenderPassAttachment (
                0,
                normalImageObj->getImageFormat(),
                normalImageObj->getImageSamplesCount(),
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );
            /* Position attachment */
            renderPassObj->addRenderPassAttachment (
                0,
                positionImageObj->getImageFormat(),
                positionImageObj->getImageSamplesCount(),
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );
            /* Color-0 attachment */
            renderPassObj->addRenderPassAttachment (
                0,
                color0ImageObj->getImageFormat(),
                color0ImageObj->getImageSamplesCount(),
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );
            /* Color-1 attachment */
            renderPassObj->addRenderPassAttachment (
                0,
                color1ImageObj->getImageFormat(),
                color1ImageObj->getImageSamplesCount(),
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );
            /* Color-2 attachment */
            renderPassObj->addRenderPassAttachment (
                0,
                color2ImageObj->getImageFormat(),
                color2ImageObj->getImageSamplesCount(),
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );
            /* Depth attachment */
            renderPassObj->addRenderPassAttachment (
                0,
                depthImageObj->getImageFormat(),
                depthImageObj->getImageSamplesCount(),
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            );
            /* Sub pass */
            auto inputAttachmentReferences   = std::vector <VkAttachmentReference> {};
            auto colorAttachmentReferences   = std::vector {
                renderPassObj->createAttachmentReference (
                    0,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                ),
                renderPassObj->createAttachmentReference (
                    1,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                ),
                renderPassObj->createAttachmentReference (
                    2,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                ),
                renderPassObj->createAttachmentReference (
                    3,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                ),
                renderPassObj->createAttachmentReference (
                    4,
                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
                )
            };
            auto depthAttachmentReference    =
                renderPassObj->createAttachmentReference (
                    5,
                    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                );
            auto resolveAttachmentReferences = std::vector <VkAttachmentReference> {};
            renderPassObj->addSubPass (
                inputAttachmentReferences,
                colorAttachmentReferences,
                &depthAttachmentReference,
                resolveAttachmentReferences
            );
            /* Dependency */
            renderPassObj->addSubPassDependency (
                0,
                VK_SUBPASS_EXTERNAL,
                0,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_SHADER_READ_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
            );
            renderPassObj->addSubPassDependency (
                0,
                VK_SUBPASS_EXTERNAL,
                0,
                VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKRenderPass> ("G", renderPassObj);
        }
        {   /* Frame buffer     [G] */
            auto renderPassObj = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass> ("G");
            auto bufferObj     = new Renderer::VKFrameBuffer (logObj, logDeviceObj, renderPassObj);
            bufferObj->initFrameBufferInfo (
                swapChainObj->getSwapChainExtent()->width,
                swapChainObj->getSwapChainExtent()->height,
                1,
                {
                    *normalImageObj->getImageView(),            /* Attachment idx 0 */
                    *positionImageObj->getImageView(),          /* Attachment idx 1 */
                    *color0ImageObj->getImageView(),            /* Attachment idx 2 */
                    *color1ImageObj->getImageView(),            /* Attachment idx 3 */
                    *color2ImageObj->getImageView(),            /* Attachment idx 4 */
                    *depthImageObj->getImageView()              /* Attachment idx 5 */
                }
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKFrameBuffer> ("G", bufferObj);
        }

        {   /* Pipeline         [G_DEFAULT] */
            auto renderPassObj = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass> ("G");
            auto pipelineObj   = new Renderer::VKPipeline (logObj, logDeviceObj, renderPassObj);
            pipelineObj->initPipelineInfo (
                 0,
                 0,
                -1,
                nullptr
            );

            auto vertexBindingDescriptions   = std::vector <VkVertexInputBindingDescription> {};
            auto vertexAttributeDescriptions = std::vector <VkVertexInputAttributeDescription> {};
            auto colorBlendAttachments       = std::vector <VkPipelineColorBlendAttachmentState> {};
            auto colorBlendConstants         = std::vector <float> {};
            auto dynamicStates               = std::vector <VkDynamicState> {};
            auto viewPorts                   = std::vector <VkViewport> {};
            auto scissors                    = std::vector <VkRect2D> {};
            auto perFrameBindingFlags        = std::vector <VkDescriptorBindingFlags> {};
            auto perFrameLayoutBindings      = std::vector <VkDescriptorSetLayoutBinding> {};
            auto otherBindingFlags           = std::vector <VkDescriptorBindingFlags> {};
            auto otherLayoutBindings         = std::vector <VkDescriptorSetLayoutBinding> {};

            {   /* Vertex input */
                vertexBindingDescriptions   = {
                    pipelineObj->createVertexBindingDescription (
                        0,
                        sizeof (Vertex),
                        VK_VERTEX_INPUT_RATE_VERTEX
                    )
                };
                vertexAttributeDescriptions = {
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
            }
            {   /* Input assembly */
                pipelineObj->createInputAssemblyState (
                    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                    VK_FALSE
                );
            }
            {   /* Shader stage */
                pipelineObj->addShaderStage (
                    VK_SHADER_STAGE_VERTEX_BIT,
                    "Build/Bin/GDefault[VERT].spv",
                    "main"
                );
                pipelineObj->addShaderStage (
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                    "Build/Bin/GDefault[FRAG].spv",
                    "main"
                );
            }
            {   /* Depth stencil */
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
            }
            {   /* Rasterization */
                pipelineObj->createRasterizationState (
                    1.0f,
                    VK_POLYGON_MODE_FILL,
                    VK_CULL_MODE_BACK_BIT,
                    VK_FRONT_FACE_COUNTER_CLOCKWISE,
                    VK_FALSE,
                    0.0f,
                    0.0f
                );
            }
            {   /* Multi sample */
                pipelineObj->createMultiSampleState (
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_FALSE,
                    0.0f
                );
            }
            {   /* Color blend */
                colorBlendAttachments = {
                    /* Normal attachment */
                    pipelineObj->createColorBlendAttachment (
                        VK_FALSE,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
                    ),
                    /* Position attachment */
                    pipelineObj->createColorBlendAttachment (
                        VK_FALSE,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
                    ),
                    /* Color-0 attachment */
                    pipelineObj->createColorBlendAttachment (
                        VK_FALSE,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
                    ),
                    /* Color-1 attachment */
                    pipelineObj->createColorBlendAttachment (
                        VK_FALSE,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
                    ),
                    /* Color-2 attachment */
                    pipelineObj->createColorBlendAttachment (
                        VK_FALSE,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
                    )
                };
                colorBlendConstants   = {
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
            }
            {   /* Dynamic state */
                dynamicStates = {
                    VK_DYNAMIC_STATE_VIEWPORT,
                    VK_DYNAMIC_STATE_SCISSOR
                };
                pipelineObj->createDynamicState (dynamicStates);
            }
            {   /* View port */
                viewPorts = {};
                scissors  = {};
                pipelineObj->createViewPortState (
                    viewPorts,
                    scissors,
                    1,
                    1
                );
            }
            {   /* Descriptor set layout */
                perFrameBindingFlags   = {
                    0
                };
                perFrameLayoutBindings = {
                    pipelineObj->createDescriptorSetLayoutBinding (
                        0,
                        1,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        VK_SHADER_STAGE_VERTEX_BIT
                    )
                };
                /* Note that, if you look at the reported value of maxPerStageDescriptorSamplers, you may find it to be
                 * too low for the application. This is because the report was run without argument buffer support turned
                 * on. This is the maximum that metal supports passing directly to a shader function. With argument
                 * buffers, (enabled by setting the MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS environment variable) the limit
                 * is much higher. Using metal argument buffers dramatically increases the number of buffers, textures
                 * and samplers that can be bound to a pipeline shader, and in most cases improves performance
                 *
                 * Additionally, we also have to enable and use VK_EXT_descriptor_indexing to get the validation layer to
                 * look at maxPerStageDescriptorUpdateAfterBindSamplers instead of maxPerStageDescriptorSamplers. An
                 * application needs to setup the following in order to achieve this
                 *
                 * (1) VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT flag for any VkDescriptorSetLayout
                 *     the descriptor is from
                 * (2) VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT flag for any VkDescriptorPool the descriptor
                 *     is allocated from
                 * (3) VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT for each binding in the VkDescriptorSetLayout that
                 *     the descriptor will use
                */
                otherBindingFlags      =  {
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT
                };
                otherLayoutBindings    =  {
                    pipelineObj->createDescriptorSetLayoutBinding (
                        0,
                        static_cast <uint32_t> (stdTexturePool.size()),
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
                    otherBindingFlags,
                    otherLayoutBindings
                );
            }
            {   /* Push constant range */
                pipelineObj->addPushConstantRange (
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    sizeof (ActiveCameraPC)
                );
            }

            collectionObj->addCollectionTypeInstance <Renderer::VKPipeline> ("G_DEFAULT", pipelineObj);
            pipelineObj->destroyShaderModules();
        }
        {   /* Descriptor pool  [G_DEFAULT] */
            auto descPoolObj = new Renderer::VKDescriptorPool (logObj, logDeviceObj);
            descPoolObj->initDescriptorPoolInfo (
                VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
                g_maxFramesInFlight + 1
            );
            descPoolObj->addDescriptorPoolSize (
                g_maxFramesInFlight,                                /* G_DEFAULT_MESH_INSTANCE_? */
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
            );
            descPoolObj->addDescriptorPoolSize (
                static_cast <uint32_t> (stdTexturePool.size()),     /* G_DEFAULT_TEXTURE_?       */
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorPool> ("G_DEFAULT", descPoolObj);
        }
        {   /* Descriptor sets  [G_DEFAULT_PER_FRAME] */
            auto pipelineObj = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>       ("G_DEFAULT");
            auto descPoolObj = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorPool> ("G_DEFAULT");
            auto descSetObj  = new Renderer::VKDescriptorSet (logObj, logDeviceObj, descPoolObj);
            descSetObj->initDescriptorSetInfo (
                g_maxFramesInFlight,
                pipelineObj->getDescriptorSetLayouts()[0]
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorSet> ("G_DEFAULT_PER_FRAME", descSetObj);

            typedef std::vector        <std::vector <VkDescriptorBufferInfo>> descriptorBufferInfosPool;
            auto descriptorImageInfos = std::vector <VkDescriptorImageInfo> {};
            std::unordered_map <uint32_t, descriptorBufferInfosPool> bindingToBufferInfosMap;

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                {   /* Binding 0 */
                    auto bufferObj             = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                        "G_DEFAULT_MESH_INSTANCE_" + std::to_string (i)
                    );
                    auto descriptorBufferInfos = std::vector <VkDescriptorBufferInfo> {
                        descSetObj->createDescriptorBufferInfo (
                            *bufferObj->getBuffer(),
                            0,
                            bufferObj->getBufferSize()
                        )
                    };
                    bindingToBufferInfosMap[0].push_back (
                        descriptorBufferInfos
                    );
                }
            }
            /* Write and update descriptor sets */
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                {   /* Binding 0 */
                    descSetObj->addWriteDescriptorSet (
                        0,
                        static_cast <uint32_t> (bindingToBufferInfosMap[0][i].size()),
                        0,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        descSetObj->getDescriptorSets()[i],
                        bindingToBufferInfosMap[0][i],
                        descriptorImageInfos
                    );
                }
            }
            descSetObj->updateDescriptorSets();
        }
        {   /* Descriptor sets  [G_DEFAULT_OTHER] */
            auto pipelineObj = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>       ("G_DEFAULT");
            auto descPoolObj = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorPool> ("G_DEFAULT");
            auto descSetObj  = new Renderer::VKDescriptorSet (logObj, logDeviceObj, descPoolObj);
            descSetObj->initDescriptorSetInfo (
                1,
                pipelineObj->getDescriptorSetLayouts()[1]
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorSet> ("G_DEFAULT_OTHER", descSetObj);

            auto descriptorBufferInfos =  std::vector <VkDescriptorBufferInfo> {};
            std::unordered_map <uint32_t, std::vector <VkDescriptorImageInfo>> bindingToImageInfosMap;

            {   /* Binding 0 */
                for (auto const& [idx, info]: stdTexturePool) {
                    auto imageObj   = collectionObj->getCollectionTypeInstance <Renderer::VKImage>   (
                        "G_DEFAULT_TEXTURE_" + std::to_string (idx)
                    );
                    auto samplerObj = collectionObj->getCollectionTypeInstance <Renderer::VKSampler> (
                        "G_DEFAULT_TEXTURE"
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
            }
            /* Write and update descriptor sets */
            {   /* Binding 0 */
                descSetObj->addWriteDescriptorSet (
                    0,
                    static_cast <uint32_t> (bindingToImageInfosMap[0].size()),
                    0,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    descSetObj->getDescriptorSets()[0],
                    descriptorBufferInfos,
                    bindingToImageInfosMap[0]
                );
            }
            descSetObj->updateDescriptorSets();
        }
    }

    void SBImpl::configRendererFPass (void) {
        auto& sceneObj             = m_sandBoxInfo.resource.sceneObj;
        auto& collectionObj        = m_sandBoxInfo.resource.collectionObj;
        auto& stdTexturePoolObj    = m_sandBoxInfo.resource.stdTexturePoolObj;
        auto& skyBoxTexturePoolObj = m_sandBoxInfo.resource.skyBoxTexturePoolObj;

        auto logObj                = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("CORE");
        auto phyDeviceObj          = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("CORE");
        auto logDeviceObj          = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("CORE");
        auto swapChainObj          = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain> ("CORE");
        auto stdTexturePool        = stdTexturePoolObj->getTexturePool();
        auto skyBoxTexturePool     = skyBoxTexturePoolObj->getTexturePool();

        {   /* Buffer           [F_LIGHT_LIGHT_INSTANCE_?] */
            auto batchingObj = sceneObj->getSystem <SYLightInstanceBatching>();
            auto instances   = batchingObj->getBatchedLightInstances();

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
                    "F_LIGHT_LIGHT_INSTANCE_" + std::to_string (i),
                    bufferObj
                );
            }
        }

        {   /* Buffer           [F_WIRE_VERTEX_STAGING] */
            auto batchingObj = sceneObj->getSystem <SYMeshBatching>();
            auto vertices    = batchingObj->getBatchedVertices (TAG_TYPE_WIRE);
            auto positions   = std::vector <glm::vec3> {};
            auto bufferObj   = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
            /* Repack vertex data since we only need the position as the vertex attribute */
            for (auto const& vertex: vertices)
                positions.push_back (vertex.meta.position);

            bufferObj->initBufferInfo (
                positions.size() * sizeof (positions[0]),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                {
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                false
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("F_WIRE_VERTEX_STAGING", bufferObj);
            bufferObj->updateBuffer (
                positions.data(),
                true
            );
        }
        {   /* Buffer           [F_WIRE_INDEX_STAGING] */
            auto batchingObj = sceneObj->getSystem <SYMeshBatching>();
            auto indices     = batchingObj->getBatchedIndices (TAG_TYPE_WIRE);
            auto bufferObj   = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
            bufferObj->initBufferInfo (
                indices.size() * sizeof (IndexType),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                {
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                false
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("F_WIRE_INDEX_STAGING", bufferObj);
            bufferObj->updateBuffer (
                indices.data(),
                true
            );
        }
        {   /* Buffer           [F_WIRE_VERTEX] */
            auto batchingObj = sceneObj->getSystem <SYMeshBatching>();
            auto vertices    = batchingObj->getBatchedVertices (TAG_TYPE_WIRE);
            auto positions   = std::vector <glm::vec3> {};
            auto bufferObj   = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);

            for (auto const& vertex: vertices)
                positions.push_back (vertex.meta.position);

            bufferObj->initBufferInfo (
                positions.size() * sizeof (positions[0]),
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                {
                    phyDeviceObj->getGraphicsQueueFamilyIdx(),
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                true
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("F_WIRE_VERTEX", bufferObj);
        }
        {   /* Buffer           [F_WIRE_INDEX] */
            auto batchingObj = sceneObj->getSystem <SYMeshBatching>();
            auto indices     = batchingObj->getBatchedIndices (TAG_TYPE_WIRE);
            auto bufferObj   = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
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

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("F_WIRE_INDEX", bufferObj);
        }
        {   /* Buffer           [F_WIRE_MESH_INSTANCE_?] */
            auto batchingObj = sceneObj->getSystem <SYWireMeshInstanceBatching>();
            auto instances   = batchingObj->getBatchedMeshInstances();

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
                    "F_WIRE_MESH_INSTANCE_" + std::to_string (i),
                    bufferObj
                );
            }
        }

        {   /* Buffer           [F_SKY_BOX_VERTEX_STAGING] */
            auto batchingObj = sceneObj->getSystem <SYMeshBatching>();
            auto vertices    = batchingObj->getBatchedVertices (TAG_TYPE_SKY_BOX);
            auto positions   = std::vector <glm::vec3> {};
            auto bufferObj   = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);

            for (auto const& vertex: vertices)
                positions.push_back (vertex.meta.position);

            bufferObj->initBufferInfo (
                positions.size() * sizeof (positions[0]),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                {
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                false
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("F_SKY_BOX_VERTEX_STAGING", bufferObj);
            bufferObj->updateBuffer (
                positions.data(),
                true
            );
        }
        {   /* Buffer           [F_SKY_BOX_INDEX_STAGING] */
            auto batchingObj = sceneObj->getSystem <SYMeshBatching>();
            auto indices     = batchingObj->getBatchedIndices (TAG_TYPE_SKY_BOX);
            auto bufferObj   = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
            bufferObj->initBufferInfo (
                indices.size() * sizeof (IndexType),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                {
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                false
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("F_SKY_BOX_INDEX_STAGING", bufferObj);
            bufferObj->updateBuffer (
                indices.data(),
                true
            );
        }
        {   /* Buffer           [F_SKY_BOX_VERTEX] */
            auto batchingObj = sceneObj->getSystem <SYMeshBatching>();
            auto vertices    = batchingObj->getBatchedVertices (TAG_TYPE_SKY_BOX);
            auto positions   = std::vector <glm::vec3> {};
            auto bufferObj   = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);

            for (auto const& vertex: vertices)
                positions.push_back (vertex.meta.position);

            bufferObj->initBufferInfo (
                positions.size() * sizeof (positions[0]),
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                {
                    phyDeviceObj->getGraphicsQueueFamilyIdx(),
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                true
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("F_SKY_BOX_VERTEX", bufferObj);
        }
        {   /* Buffer           [F_SKY_BOX_INDEX] */
            auto batchingObj = sceneObj->getSystem <SYMeshBatching>();
            auto indices     = batchingObj->getBatchedIndices (TAG_TYPE_SKY_BOX);
            auto bufferObj   = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
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

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("F_SKY_BOX_INDEX", bufferObj);
        }
        {   /* Buffer           [F_SKY_BOX_MESH_INSTANCE_?] */
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
                    "F_SKY_BOX_MESH_INSTANCE_" + std::to_string (i),
                    bufferObj
                );
            }
        }
        {   /* Buffer           [F_SKY_BOX_TEXTURE_STAGING_?] */
            for (auto const& [idx, info]: skyBoxTexturePool) {
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
                    "F_SKY_BOX_TEXTURE_STAGING_" + std::to_string (idx),
                    bufferObj
                );
                bufferObj->updateBuffer (
                    info.resource.data,
                    true
                );
                skyBoxTexturePoolObj->destroyImage (idx);
            }
        }

        {   /* Buffer           [F_DEFAULT_VERTEX_STAGING] */
            auto batchingObj = sceneObj->getSystem <SYMeshBatching>();
            auto vertices    = batchingObj->getBatchedVertices (TAG_TYPE_STD_ALPHA);
            auto bufferObj   = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
            bufferObj->initBufferInfo (
                vertices.size() * sizeof (Vertex),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                {
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                false
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("F_DEFAULT_VERTEX_STAGING", bufferObj);
            bufferObj->updateBuffer (
                vertices.data(),
                true
            );
        }
        {   /* Buffer           [F_DEFAULT_INDEX_STAGING] */
            auto batchingObj = sceneObj->getSystem <SYMeshBatching>();
            auto indices     = batchingObj->getBatchedIndices (TAG_TYPE_STD_ALPHA);
            auto bufferObj   = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
            bufferObj->initBufferInfo (
                indices.size() * sizeof (IndexType),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                {
                    phyDeviceObj->getTransferQueueFamilyIdx()
                },
                false
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("F_DEFAULT_INDEX_STAGING", bufferObj);
            bufferObj->updateBuffer (
                indices.data(),
                true
            );
        }
        {   /* Buffer           [F_DEFAULT_VERTEX] */
            auto batchingObj = sceneObj->getSystem <SYMeshBatching>();
            auto vertices    = batchingObj->getBatchedVertices (TAG_TYPE_STD_ALPHA);
            auto bufferObj   = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
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

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("F_DEFAULT_VERTEX", bufferObj);
        }
        {   /* Buffer           [F_DEFAULT_INDEX] */
            auto batchingObj = sceneObj->getSystem <SYMeshBatching>();
            auto indices     = batchingObj->getBatchedIndices (TAG_TYPE_STD_ALPHA);
            auto bufferObj   = new Renderer::VKBuffer (logObj, phyDeviceObj, logDeviceObj);
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

            collectionObj->addCollectionTypeInstance <Renderer::VKBuffer> ("F_DEFAULT_INDEX", bufferObj);
        }
        {   /* Buffer           [F_DEFAULT_MESH_INSTANCE_?] */
            auto batchingObj = sceneObj->getSystem <SYStdMeshInstanceBatching>();
            auto instances   = batchingObj->getBatchedMeshInstances (TAG_TYPE_STD_ALPHA);

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
                    "F_DEFAULT_MESH_INSTANCE_" + std::to_string (i),
                    bufferObj
                );
            }
        }

        {   /* Image            [F_SKY_BOX_TEXTURE] */
            auto imageObj = new Renderer::VKImage (logObj, phyDeviceObj, logDeviceObj);
            imageObj->initImageInfo (
                static_cast <uint32_t> (skyBoxTexturePool[0].meta.width),
                static_cast <uint32_t> (skyBoxTexturePool[0].meta.height),
                1,
                0,
                6,
                VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_FORMAT_R8G8B8A8_SRGB,
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

            collectionObj->addCollectionTypeInstance <Renderer::VKImage> ("F_SKY_BOX_TEXTURE", imageObj);
        }
        {   /* Sampler          [F_LIGHT_GBUFFER] */
            auto samplerObj = new Renderer::VKSampler (logObj, logDeviceObj);
            samplerObj->initSamplerInfo (
                0.0f,
                0.0f,
                VK_LOD_CLAMP_NONE,
                0.0f,
                VK_FILTER_NEAREST,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
                VK_SAMPLER_MIPMAP_MODE_NEAREST,
                VK_BORDER_COLOR_INT_OPAQUE_WHITE,
                VK_TRUE
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKSampler> ("F_LIGHT_GBUFFER", samplerObj);
        }
        {   /* Sampler          [F_SKY_BOX_TEXTURE] */
            auto samplerObj = new Renderer::VKSampler (logObj, logDeviceObj);
            /* Note that, we are setting the sampler address mode to VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE since texture
             * coordinates that are exactly between two faces may not hit an exact face (due to hardware limitations) so
             * by using VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, the sampler always returns their edge values whenever we
             * sample between faces
            */
            samplerObj->initSamplerInfo (
                0.0f,
                0.0f,
                VK_LOD_CLAMP_NONE,
                0.0f,
                VK_FILTER_NEAREST,
                VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                VK_SAMPLER_MIPMAP_MODE_NEAREST,
                VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                VK_TRUE
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKSampler> ("F_SKY_BOX_TEXTURE", samplerObj);
        }

        auto depthImageObj     = collectionObj->getCollectionTypeInstance <Renderer::VKImage>   ("G_DEFAULT_DEPTH");
        auto gBufferSamplerObj = collectionObj->getCollectionTypeInstance <Renderer::VKSampler> ("F_LIGHT_GBUFFER");
        auto skyBoxSamplerObj  = collectionObj->getCollectionTypeInstance <Renderer::VKSampler> ("F_SKY_BOX_TEXTURE");

        {   /* Render pass      [F] */
            auto renderPassObj = new Renderer::VKRenderPass (logObj, logDeviceObj);
            renderPassObj->initRenderPassInfo();

            /* Color attachment */
            renderPassObj->addRenderPassAttachment (
                0,
                swapChainObj->getSwapChainFormat(),
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            );
            /* Depth attachment */
            renderPassObj->addRenderPassAttachment (
                0,
                depthImageObj->getImageFormat(),
                depthImageObj->getImageSamplesCount(),
                VK_ATTACHMENT_LOAD_OP_LOAD,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            );
            /* Sub pass */
            auto inputAttachmentReferences   = std::vector <VkAttachmentReference> {};
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
            auto resolveAttachmentReferences = std::vector <VkAttachmentReference> {};
            renderPassObj->addSubPass (
                inputAttachmentReferences,
                colorAttachmentReferences,
                &depthAttachmentReference,
                resolveAttachmentReferences
            );
            /* Dependency */
            renderPassObj->addSubPassDependency (
                0,
                VK_SUBPASS_EXTERNAL,
                0,
                VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_ACCESS_SHADER_READ_BIT
            );
            renderPassObj->addSubPassDependency (
                0,
                VK_SUBPASS_EXTERNAL,
                0,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_ACCESS_SHADER_READ_BIT
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
            renderPassObj->addSubPassDependency (
                0,
                VK_SUBPASS_EXTERNAL,
                0,
                VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKRenderPass> ("F", renderPassObj);
        }
        {   /* Frame buffer     [F_?] */
            auto renderPassObj = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass> ("F");

            for (uint32_t i = 0; i < swapChainObj->getSwapChainImagesCount(); i++) {
                auto swapChainImageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage> (
                    "SWAP_CHAIN_" + std::to_string (i)
                );
                auto bufferObj         = new Renderer::VKFrameBuffer (logObj, logDeviceObj, renderPassObj);
                bufferObj->initFrameBufferInfo (
                    swapChainObj->getSwapChainExtent()->width,
                    swapChainObj->getSwapChainExtent()->height,
                    1,
                    {
                        *swapChainImageObj->getImageView(),     /* Attachment idx 0 */
                        *depthImageObj->getImageView()          /* Attachment idx 1 */
                    }
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKFrameBuffer> (
                    "F_" + std::to_string (i),
                    bufferObj
                );
            }
        }

        {   /* Pipeline         [F_LIGHT] */
            auto batchingObj          = sceneObj->getSystem <SYLightInstanceBatching>();
            auto lightTypeOffsets     = batchingObj->getLightTypeOffsets();
            uint32_t otherLightsCount = lightTypeOffsets->pointLightsOffset;
            uint32_t pointLightsCount = lightTypeOffsets->lightsCount - lightTypeOffsets->pointLightsOffset;

            auto renderPassObj        = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass> ("F");
            auto pipelineObj          = new Renderer::VKPipeline (logObj, logDeviceObj, renderPassObj);
            pipelineObj->initPipelineInfo (
                 VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
                 0,
                -1,
                nullptr
            );

            auto vertexBindingDescriptions   = std::vector <VkVertexInputBindingDescription> {};
            auto vertexAttributeDescriptions = std::vector <VkVertexInputAttributeDescription> {};
            auto colorBlendAttachments       = std::vector <VkPipelineColorBlendAttachmentState> {};
            auto colorBlendConstants         = std::vector <float> {};
            auto dynamicStates               = std::vector <VkDynamicState> {};
            auto viewPorts                   = std::vector <VkViewport> {};
            auto scissors                    = std::vector <VkRect2D> {};
            auto perFrameBindingFlags        = std::vector <VkDescriptorBindingFlags> {};
            auto perFrameLayoutBindings      = std::vector <VkDescriptorSetLayoutBinding> {};
            auto otherBindingFlags           = std::vector <VkDescriptorBindingFlags> {};
            auto otherLayoutBindings         = std::vector <VkDescriptorSetLayoutBinding> {};

            {   /* Vertex input */
                vertexBindingDescriptions   = {};
                vertexAttributeDescriptions = {};
                pipelineObj->createVertexInputState (
                    vertexBindingDescriptions,
                    vertexAttributeDescriptions
                );
            }
            {   /* Input assembly */
                pipelineObj->createInputAssemblyState (
                    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                    VK_FALSE
                );
            }
            {   /* Shader stage */
                pipelineObj->addShaderStage (
                    VK_SHADER_STAGE_VERTEX_BIT,
                    "Build/Bin/Light[VERT].spv",
                    "main"
                );
                pipelineObj->addShaderStage (
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                    "Build/Bin/Light[FRAG].spv",
                    "main"
                );
            }
            {   /* Depth stencil */
                pipelineObj->createDepthStencilState (
                    VK_FALSE,
                    VK_FALSE,
                    VK_FALSE,
                    VK_FALSE,
                    VK_COMPARE_OP_NEVER,
                    0.0f,
                    1.0f,
                    {},
                    {}
                );
            }
            {   /* Rasterization */
                pipelineObj->createRasterizationState (
                    1.0f,
                    VK_POLYGON_MODE_FILL,
                    VK_CULL_MODE_BACK_BIT,
                    VK_FRONT_FACE_COUNTER_CLOCKWISE,
                    VK_FALSE,
                    0.0f,
                    0.0f
                );
            }
            {   /* Multi sample */
                pipelineObj->createMultiSampleState (
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_FALSE,
                    0.0f
                );
            }
            {   /* Color blend */
                colorBlendAttachments = {
                    pipelineObj->createColorBlendAttachment (
                        VK_FALSE,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
                    )
                };
                colorBlendConstants   = {
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
            }
            {   /* Dynamic state */
                dynamicStates = {
                    VK_DYNAMIC_STATE_VIEWPORT,
                    VK_DYNAMIC_STATE_SCISSOR
                };
                pipelineObj->createDynamicState (dynamicStates);
            }
            {   /* View port */
                viewPorts = {};
                scissors  = {};
                pipelineObj->createViewPortState (
                    viewPorts,
                    scissors,
                    1,
                    1
                );
            }
            {   /* Descriptor set layout */
                perFrameBindingFlags   = {
                    0
                };
                perFrameLayoutBindings = {
                    pipelineObj->createDescriptorSetLayoutBinding (
                        0,
                        1,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        VK_SHADER_STAGE_FRAGMENT_BIT
                    )
                };
                otherBindingFlags      = {
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT,
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT,
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT,
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT,
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT,
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT,
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT,
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT
                };
                otherLayoutBindings    = {
                    pipelineObj->createDescriptorSetLayoutBinding (
                        0,
                        otherLightsCount,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT
                    ),
                    pipelineObj->createDescriptorSetLayoutBinding (
                        1,
                        pointLightsCount,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT
                    ),
                    pipelineObj->createDescriptorSetLayoutBinding (
                        2,
                        1,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT
                    ),
                    pipelineObj->createDescriptorSetLayoutBinding (
                        3,
                        1,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT
                    ),
                    pipelineObj->createDescriptorSetLayoutBinding (
                        4,
                        1,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT
                    ),
                    pipelineObj->createDescriptorSetLayoutBinding (
                        5,
                        1,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT
                    ),
                    pipelineObj->createDescriptorSetLayoutBinding (
                        6,
                        1,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT
                    ),
                    pipelineObj->createDescriptorSetLayoutBinding (
                        7,
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
                    otherBindingFlags,
                    otherLayoutBindings
                );
            }
            {   /* Push constant range */
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
            }

            collectionObj->addCollectionTypeInstance <Renderer::VKPipeline> ("F_LIGHT", pipelineObj);
            pipelineObj->destroyShaderModules();
        }
        {   /* Descriptor pool  [F_LIGHT] */
            auto batchingObj          = sceneObj->getSystem <SYLightInstanceBatching>();
            auto lightTypeOffsets     = batchingObj->getLightTypeOffsets();
            uint32_t otherLightsCount = lightTypeOffsets->pointLightsOffset;
            uint32_t pointLightsCount = lightTypeOffsets->lightsCount - lightTypeOffsets->pointLightsOffset;

            auto descPoolObj          = new Renderer::VKDescriptorPool (logObj, logDeviceObj);
            descPoolObj->initDescriptorPoolInfo (
                VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
                g_maxFramesInFlight + 1
            );
            descPoolObj->addDescriptorPoolSize (
                g_maxFramesInFlight,                            /* F_LIGHT_LIGHT_INSTANCE_? */
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
            );
            descPoolObj->addDescriptorPoolSize (
                otherLightsCount +                              /* S_DEFAULT_DEPTH_?        */
                pointLightsCount +                              /* F_LIGHT_DEPTH_CUBE_?     */
                1                +                              /* G_DEFAULT_NORMAL         */
                1                +                              /* G_DEFAULT_POSITION       */
                1                +                              /* G_DEFAULT_COLOR_0        */
                1                +                              /* G_DEFAULT_COLOR_1        */
                1                +                              /* G_DEFAULT_COLOR_2        */
                1,                                              /* F_SKY_BOX_TEXTURE        */
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorPool> ("F_LIGHT", descPoolObj);
        }
        {   /* Descriptor sets  [F_LIGHT_PER_FRAME] */
            auto pipelineObj = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>       ("F_LIGHT");
            auto descPoolObj = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorPool> ("F_LIGHT");
            auto descSetObj  = new Renderer::VKDescriptorSet (logObj, logDeviceObj, descPoolObj);
            descSetObj->initDescriptorSetInfo (
                g_maxFramesInFlight,
                pipelineObj->getDescriptorSetLayouts()[0]
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorSet> ("F_LIGHT_PER_FRAME", descSetObj);

            typedef std::vector        <std::vector <VkDescriptorBufferInfo>> descriptorBufferInfosPool;
            auto descriptorImageInfos = std::vector <VkDescriptorImageInfo> {};
            std::unordered_map <uint32_t, descriptorBufferInfosPool> bindingToBufferInfosMap;

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                {   /* Binding 0 */
                    auto bufferObj             = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                        "F_LIGHT_LIGHT_INSTANCE_" + std::to_string (i)
                    );
                    auto descriptorBufferInfos = std::vector <VkDescriptorBufferInfo> {
                        descSetObj->createDescriptorBufferInfo (
                            *bufferObj->getBuffer(),
                            0,
                            bufferObj->getBufferSize()
                        )
                    };
                    bindingToBufferInfosMap[0].push_back (
                        descriptorBufferInfos
                    );
                }
            }
            /* Write and update descriptor sets */
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                {   /* Binding 0 */
                    descSetObj->addWriteDescriptorSet (
                        0,
                        static_cast <uint32_t> (bindingToBufferInfosMap[0][i].size()),
                        0,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        descSetObj->getDescriptorSets()[i],
                        bindingToBufferInfosMap[0][i],
                        descriptorImageInfos
                    );
                }
            }
            descSetObj->updateDescriptorSets();
        }
        {   /* Descriptor sets  [F_LIGHT_OTHER] */
            auto batchingObj          = sceneObj->getSystem <SYLightInstanceBatching>();
            auto lightTypeOffsets     = batchingObj->getLightTypeOffsets();
            uint32_t otherLightsCount = lightTypeOffsets->pointLightsOffset;
            uint32_t pointLightsCount = lightTypeOffsets->lightsCount - lightTypeOffsets->pointLightsOffset;

            auto pipelineObj          = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>       ("F_LIGHT");
            auto descPoolObj          = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorPool> ("F_LIGHT");
            auto descSetObj           = new Renderer::VKDescriptorSet (logObj, logDeviceObj, descPoolObj);
            descSetObj->initDescriptorSetInfo (
                1,
                pipelineObj->getDescriptorSetLayouts()[1]
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorSet> ("F_LIGHT_OTHER", descSetObj);

            auto descriptorBufferInfos =  std::vector <VkDescriptorBufferInfo> {};
            std::unordered_map <uint32_t, std::vector <VkDescriptorImageInfo>> bindingToImageInfosMap;

            {   /* Binding 0 */
                for (uint32_t i = 0; i < otherLightsCount; i++) {
                    auto imageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage> (
                        "S_DEFAULT_DEPTH_" + std::to_string (i)
                    );
                    bindingToImageInfosMap[0].push_back (
                        descSetObj->createDescriptorImageInfo (
                            *gBufferSamplerObj->getSampler(),
                            *imageObj->getImageView(),
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                        )
                    );
                }
            }
            {   /* Binding 1 */
                for (uint32_t i = 0; i < pointLightsCount; i++) {
                    auto imageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage> (
                        "F_LIGHT_DEPTH_CUBE_" + std::to_string (i)
                    );
                    bindingToImageInfosMap[1].push_back (
                        descSetObj->createDescriptorImageInfo (
                            *skyBoxSamplerObj->getSampler(),
                            *imageObj->getImageView(),
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                        )
                    );
                }
            }
            {   /* Binding 2 */
                auto imageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage> ("G_DEFAULT_NORMAL");

                bindingToImageInfosMap[2].push_back (
                    descSetObj->createDescriptorImageInfo (
                        *gBufferSamplerObj->getSampler(),
                        *imageObj->getImageView(),
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    )
                );
            }
            {   /* Binding 3 */
                auto imageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage> ("G_DEFAULT_POSITION");

                bindingToImageInfosMap[3].push_back (
                    descSetObj->createDescriptorImageInfo (
                        *gBufferSamplerObj->getSampler(),
                        *imageObj->getImageView(),
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    )
                );
            }
            {   /* Binding 4 */
                auto imageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage> ("G_DEFAULT_COLOR_0");

                bindingToImageInfosMap[4].push_back (
                    descSetObj->createDescriptorImageInfo (
                        *gBufferSamplerObj->getSampler(),
                        *imageObj->getImageView(),
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    )
                );
            }
            {   /* Binding 5 */
                auto imageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage> ("G_DEFAULT_COLOR_1");

                bindingToImageInfosMap[5].push_back (
                    descSetObj->createDescriptorImageInfo (
                        *gBufferSamplerObj->getSampler(),
                        *imageObj->getImageView(),
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    )
                );
            }
            {   /* Binding 6 */
                auto imageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage> ("G_DEFAULT_COLOR_2");

                bindingToImageInfosMap[6].push_back (
                    descSetObj->createDescriptorImageInfo (
                        *gBufferSamplerObj->getSampler(),
                        *imageObj->getImageView(),
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    )
                );
            }
            {   /* Binding 7 */
                auto imageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage> ("F_SKY_BOX_TEXTURE");

                bindingToImageInfosMap[7].push_back (
                    descSetObj->createDescriptorImageInfo (
                        *skyBoxSamplerObj->getSampler(),
                        *imageObj->getImageView(),
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    )
                );
            }
            /* Write and update descriptor sets */
            {   /* Binding 0 */
                descSetObj->addWriteDescriptorSet (
                    0,
                    static_cast <uint32_t> (bindingToImageInfosMap[0].size()),
                    0,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    descSetObj->getDescriptorSets()[0],
                    descriptorBufferInfos,
                    bindingToImageInfosMap[0]
                );
            }
            {   /* Binding 1 */
                descSetObj->addWriteDescriptorSet (
                    1,
                    static_cast <uint32_t> (bindingToImageInfosMap[1].size()),
                    0,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    descSetObj->getDescriptorSets()[0],
                    descriptorBufferInfos,
                    bindingToImageInfosMap[1]
                );
            }
            {   /* Binding 2 */
                descSetObj->addWriteDescriptorSet (
                    2,
                    static_cast <uint32_t> (bindingToImageInfosMap[2].size()),
                    0,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    descSetObj->getDescriptorSets()[0],
                    descriptorBufferInfos,
                    bindingToImageInfosMap[2]
                );
            }
            {   /* Binding 3 */
                descSetObj->addWriteDescriptorSet (
                    3,
                    static_cast <uint32_t> (bindingToImageInfosMap[3].size()),
                    0,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    descSetObj->getDescriptorSets()[0],
                    descriptorBufferInfos,
                    bindingToImageInfosMap[3]
                );
            }
            {   /* Binding 4 */
                descSetObj->addWriteDescriptorSet (
                    4,
                    static_cast <uint32_t> (bindingToImageInfosMap[4].size()),
                    0,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    descSetObj->getDescriptorSets()[0],
                    descriptorBufferInfos,
                    bindingToImageInfosMap[4]
                );
            }
            {   /* Binding 5 */
                descSetObj->addWriteDescriptorSet (
                    5,
                    static_cast <uint32_t> (bindingToImageInfosMap[5].size()),
                    0,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    descSetObj->getDescriptorSets()[0],
                    descriptorBufferInfos,
                    bindingToImageInfosMap[5]
                );
            }
            {   /* Binding 6 */
                descSetObj->addWriteDescriptorSet (
                    6,
                    static_cast <uint32_t> (bindingToImageInfosMap[6].size()),
                    0,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    descSetObj->getDescriptorSets()[0],
                    descriptorBufferInfos,
                    bindingToImageInfosMap[6]
                );
            }
            {   /* Binding 7 */
                descSetObj->addWriteDescriptorSet (
                    7,
                    static_cast <uint32_t> (bindingToImageInfosMap[7].size()),
                    0,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    descSetObj->getDescriptorSets()[0],
                    descriptorBufferInfos,
                    bindingToImageInfosMap[7]
                );
            }
            descSetObj->updateDescriptorSets();
        }

        {   /* Pipeline         [F_WIRE] */
            auto renderPassObj   = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass> ("F");
            auto basePipelineObj = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>   ("F_LIGHT");
            auto pipelineObj     = new Renderer::VKPipeline (logObj, logDeviceObj, renderPassObj);
            pipelineObj->initPipelineInfo (
                 VK_PIPELINE_CREATE_DERIVATIVE_BIT,
                 0,
                -1,
                *basePipelineObj->getPipeline()
            );

            auto vertexBindingDescriptions   = std::vector <VkVertexInputBindingDescription> {};
            auto vertexAttributeDescriptions = std::vector <VkVertexInputAttributeDescription> {};
            auto colorBlendAttachments       = std::vector <VkPipelineColorBlendAttachmentState> {};
            auto colorBlendConstants         = std::vector <float> {};
            auto dynamicStates               = std::vector <VkDynamicState> {};
            auto viewPorts                   = std::vector <VkViewport> {};
            auto scissors                    = std::vector <VkRect2D> {};
            auto perFrameBindingFlags        = std::vector <VkDescriptorBindingFlags> {};
            auto perFrameLayoutBindings      = std::vector <VkDescriptorSetLayoutBinding> {};

            {   /* Vertex input */
                vertexBindingDescriptions   = {
                    pipelineObj->createVertexBindingDescription (
                        0,
                        sizeof (glm::vec3),
                        VK_VERTEX_INPUT_RATE_VERTEX
                    )
                };
                vertexAttributeDescriptions = {
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
            }
            {   /* Input assembly */
                pipelineObj->createInputAssemblyState (
                    VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
                    VK_FALSE
                );
            }
            {   /* Shader stage */
                pipelineObj->addShaderStage (
                    VK_SHADER_STAGE_VERTEX_BIT,
                    "Build/Bin/Wire[VERT].spv",
                    "main"
                );
                pipelineObj->addShaderStage (
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                    "Build/Bin/Wire[FRAG].spv",
                    "main"
                );
            }
            {   /* Depth stencil */
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
            }
            {   /* Rasterization */
                /* Note that, wideLines (VkPhysicalDeviceFeatures) specifies whether lines with width other than 1.0 are
                 * supported. If not enabled, the lineWidth member must be 1.0 unless the VK_DYNAMIC_STATE_LINE_WIDTH
                 * dynamic state is enabled, in which case the lineWidth parameter to vkCmdSetLineWidth must be 1.0
                 *
                 * However, as of when this code is written, there are no native vulkan drivers on MacOS nor iOS, only
                 * emulation through MoltenVK which translates vulkan API calls to Metal API calls. Since Metal does not
                 * support wideLines, the features is reported as not supported in VkPhysicalDeviceFeatures
                */
                pipelineObj->createRasterizationState (
                    1.0f,
                    VK_POLYGON_MODE_LINE,
                    VK_CULL_MODE_NONE,
                    VK_FRONT_FACE_COUNTER_CLOCKWISE,
                    VK_FALSE,
                    0.0f,
                    0.0f
                );
            }
            {   /* Multi sample */
                pipelineObj->createMultiSampleState (
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_FALSE,
                    0.0f
                );
            }
            {   /* Color blend */
                colorBlendAttachments = {
                    pipelineObj->createColorBlendAttachment (
                        VK_FALSE,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
                    )
                };
                colorBlendConstants   = {
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
            }
            {   /* Dynamic state */
                dynamicStates = {
                    VK_DYNAMIC_STATE_VIEWPORT,
                    VK_DYNAMIC_STATE_SCISSOR
                };
                pipelineObj->createDynamicState (dynamicStates);
            }
            {   /* View port */
                viewPorts = {};
                scissors  = {};
                pipelineObj->createViewPortState (
                    viewPorts,
                    scissors,
                    1,
                    1
                );
            }
            {   /* Descriptor set layout */
                perFrameBindingFlags   = {
                    0
                };
                perFrameLayoutBindings = {
                    pipelineObj->createDescriptorSetLayoutBinding (
                        0,
                        1,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        VK_SHADER_STAGE_VERTEX_BIT
                    )
                };
                /* Layout 0 */
                pipelineObj->addDescriptorSetLayout (
                    0,
                    perFrameBindingFlags,
                    perFrameLayoutBindings
                );
            }
            {   /* Push constant range */
                pipelineObj->addPushConstantRange (
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    sizeof (ActiveCameraPC)
                );
            }

            collectionObj->addCollectionTypeInstance <Renderer::VKPipeline> ("F_WIRE", pipelineObj);
            pipelineObj->destroyShaderModules();
        }
        {   /* Descriptor pool  [F_WIRE] */
            auto descPoolObj = new Renderer::VKDescriptorPool (logObj, logDeviceObj);
            descPoolObj->initDescriptorPoolInfo (
                0,
                g_maxFramesInFlight
            );
            descPoolObj->addDescriptorPoolSize (
                g_maxFramesInFlight,                            /* F_WIRE_MESH_INSTANCE_? */
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorPool> ("F_WIRE", descPoolObj);
        }
        {   /* Descriptor sets  [F_WIRE_PER_FRAME] */
            auto pipelineObj = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>       ("F_WIRE");
            auto descPoolObj = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorPool> ("F_WIRE");
            auto descSetObj  = new Renderer::VKDescriptorSet (logObj, logDeviceObj, descPoolObj);
            descSetObj->initDescriptorSetInfo (
                g_maxFramesInFlight,
                pipelineObj->getDescriptorSetLayouts()[0]
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorSet> ("F_WIRE_PER_FRAME", descSetObj);

            typedef std::vector        <std::vector <VkDescriptorBufferInfo>> descriptorBufferInfosPool;
            auto descriptorImageInfos = std::vector <VkDescriptorImageInfo> {};
            std::unordered_map <uint32_t, descriptorBufferInfosPool> bindingToBufferInfosMap;

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                {   /* Binding 0 */
                    auto bufferObj             = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                        "F_WIRE_MESH_INSTANCE_" + std::to_string (i)
                    );
                    auto descriptorBufferInfos = std::vector <VkDescriptorBufferInfo> {
                        descSetObj->createDescriptorBufferInfo (
                            *bufferObj->getBuffer(),
                            0,
                            bufferObj->getBufferSize()
                        )
                    };
                    bindingToBufferInfosMap[0].push_back (
                        descriptorBufferInfos
                    );
                }
            }
            /* Write and update descriptor sets */
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                {   /* Binding 0 */
                    descSetObj->addWriteDescriptorSet (
                        0,
                        static_cast <uint32_t> (bindingToBufferInfosMap[0][i].size()),
                        0,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        descSetObj->getDescriptorSets()[i],
                        bindingToBufferInfosMap[0][i],
                        descriptorImageInfos
                    );
                }
            }
            descSetObj->updateDescriptorSets();
        }

        {   /* Pipeline         [F_SKY_BOX] */
            auto renderPassObj   = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass> ("F");
            auto basePipelineObj = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>   ("F_LIGHT");
            auto pipelineObj     = new Renderer::VKPipeline (logObj, logDeviceObj, renderPassObj);
            pipelineObj->initPipelineInfo (
                 VK_PIPELINE_CREATE_DERIVATIVE_BIT,
                 0,
                -1,
                *basePipelineObj->getPipeline()
            );

            auto vertexBindingDescriptions   = std::vector <VkVertexInputBindingDescription> {};
            auto vertexAttributeDescriptions = std::vector <VkVertexInputAttributeDescription> {};
            auto colorBlendAttachments       = std::vector <VkPipelineColorBlendAttachmentState> {};
            auto colorBlendConstants         = std::vector <float> {};
            auto dynamicStates               = std::vector <VkDynamicState> {};
            auto viewPorts                   = std::vector <VkViewport> {};
            auto scissors                    = std::vector <VkRect2D> {};
            auto perFrameBindingFlags        = std::vector <VkDescriptorBindingFlags> {};
            auto perFrameLayoutBindings      = std::vector <VkDescriptorSetLayoutBinding> {};
            auto otherBindingFlags           = std::vector <VkDescriptorBindingFlags> {};
            auto otherLayoutBindings         = std::vector <VkDescriptorSetLayoutBinding> {};

            {   /* Vertex input */
                vertexBindingDescriptions   = {
                    pipelineObj->createVertexBindingDescription (
                        0,
                        sizeof (glm::vec3),
                        VK_VERTEX_INPUT_RATE_VERTEX
                    )
                };
                vertexAttributeDescriptions = {
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
            }
            {   /* Input assembly */
                pipelineObj->createInputAssemblyState (
                    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                    VK_FALSE
                );
            }
            {   /* Shader stage */
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
            }
            {   /* Depth stencil */
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
            }
            {   /* Rasterization */
                pipelineObj->createRasterizationState (
                    1.0f,
                    VK_POLYGON_MODE_FILL,
                    VK_CULL_MODE_FRONT_BIT,
                    VK_FRONT_FACE_COUNTER_CLOCKWISE,
                    VK_FALSE,
                    0.0f,
                    0.0f
                );
            }
            {   /* Multi sample */
                pipelineObj->createMultiSampleState (
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_FALSE,
                    0.0f
                );
            }
            {   /* Color blend */
                colorBlendAttachments = {
                    pipelineObj->createColorBlendAttachment (
                        VK_FALSE,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
                    )
                };
                colorBlendConstants   = {
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
            }
            {   /* Dynamic state */
                dynamicStates = {
                    VK_DYNAMIC_STATE_VIEWPORT,
                    VK_DYNAMIC_STATE_SCISSOR
                };
                pipelineObj->createDynamicState (dynamicStates);
            }
            {   /* View port */
                viewPorts = {};
                scissors  = {};
                pipelineObj->createViewPortState (
                    viewPorts,
                    scissors,
                    1,
                    1
                );
            }
            {   /* Descriptor set layout */
                perFrameBindingFlags   = {
                    0
                };
                perFrameLayoutBindings = {
                    pipelineObj->createDescriptorSetLayoutBinding (
                        0,
                        1,
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_VERTEX_BIT
                    )
                };
                otherBindingFlags      = {
                    0
                };
                otherLayoutBindings    = {
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
                    otherBindingFlags,
                    otherLayoutBindings
                );
            }
            {   /* Push constant range */
                pipelineObj->addPushConstantRange (
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    sizeof (ActiveCameraPC)
                );
            }

            collectionObj->addCollectionTypeInstance <Renderer::VKPipeline> ("F_SKY_BOX", pipelineObj);
            pipelineObj->destroyShaderModules();
        }
        {   /* Descriptor pool  [F_SKY_BOX] */
            auto descPoolObj = new Renderer::VKDescriptorPool (logObj, logDeviceObj);
            descPoolObj->initDescriptorPoolInfo (
                0,
                g_maxFramesInFlight + 1
            );
            descPoolObj->addDescriptorPoolSize (
                g_maxFramesInFlight,                            /* F_SKY_BOX_MESH_INSTANCE_? */
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
            );
            descPoolObj->addDescriptorPoolSize (
                1,                                              /* F_SKY_BOX_TEXTURE         */
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorPool> ("F_SKY_BOX", descPoolObj);
        }
        {   /* Descriptor sets  [F_SKY_BOX_PER_FRAME] */
            auto pipelineObj = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>       ("F_SKY_BOX");
            auto descPoolObj = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorPool> ("F_SKY_BOX");
            auto descSetObj  = new Renderer::VKDescriptorSet (logObj, logDeviceObj, descPoolObj);
            descSetObj->initDescriptorSetInfo (
                g_maxFramesInFlight,
                pipelineObj->getDescriptorSetLayouts()[0]
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorSet> ("F_SKY_BOX_PER_FRAME", descSetObj);

            typedef std::vector        <std::vector <VkDescriptorBufferInfo>> descriptorBufferInfosPool;
            auto descriptorImageInfos = std::vector <VkDescriptorImageInfo> {};
            std::unordered_map <uint32_t, descriptorBufferInfosPool> bindingToBufferInfosMap;

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                {   /* Binding 0 */
                    auto bufferObj             = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                        "F_SKY_BOX_MESH_INSTANCE_" + std::to_string (i)
                    );
                    auto descriptorBufferInfos = std::vector <VkDescriptorBufferInfo> {
                        descSetObj->createDescriptorBufferInfo (
                            *bufferObj->getBuffer(),
                            0,
                            bufferObj->getBufferSize()
                        )
                    };
                    bindingToBufferInfosMap[0].push_back (
                        descriptorBufferInfos
                    );
                }
            }
            /* Write and update descriptor sets */
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                {   /* Binding 0 */
                    descSetObj->addWriteDescriptorSet (
                        0,
                        static_cast <uint32_t> (bindingToBufferInfosMap[0][i].size()),
                        0,
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        descSetObj->getDescriptorSets()[i],
                        bindingToBufferInfosMap[0][i],
                        descriptorImageInfos
                    );
                }
            }
            descSetObj->updateDescriptorSets();
        }
        {   /* Descriptor sets  [F_SKY_BOX_OTHER] */
            auto pipelineObj = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>       ("F_SKY_BOX");
            auto descPoolObj = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorPool> ("F_SKY_BOX");
            auto descSetObj  = new Renderer::VKDescriptorSet (logObj, logDeviceObj, descPoolObj);
            descSetObj->initDescriptorSetInfo (
                1,
                pipelineObj->getDescriptorSetLayouts()[1]
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorSet> ("F_SKY_BOX_OTHER", descSetObj);

            auto descriptorBufferInfos =  std::vector <VkDescriptorBufferInfo> {};
            std::unordered_map <uint32_t, std::vector <VkDescriptorImageInfo>> bindingToImageInfosMap;

            {   /* Binding 0 */
                auto imageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage> ("F_SKY_BOX_TEXTURE");

                bindingToImageInfosMap[0].push_back (
                    descSetObj->createDescriptorImageInfo (
                        *skyBoxSamplerObj->getSampler(),
                        *imageObj->getImageView(),
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    )
                );
            }
            /* Write and update descriptor sets */
            {   /* Binding 0 */
                descSetObj->addWriteDescriptorSet (
                    0,
                    static_cast <uint32_t> (bindingToImageInfosMap[0].size()),
                    0,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    descSetObj->getDescriptorSets()[0],
                    descriptorBufferInfos,
                    bindingToImageInfosMap[0]
                );
            }
            descSetObj->updateDescriptorSets();
        }

        {   /* Pipeline         [F_DEFAULT] */
            auto renderPassObj   = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass> ("F");
            auto basePipelineObj = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>   ("F_LIGHT");
            auto pipelineObj     = new Renderer::VKPipeline (logObj, logDeviceObj, renderPassObj);
            pipelineObj->initPipelineInfo (
                 VK_PIPELINE_CREATE_DERIVATIVE_BIT,
                 0,
                -1,
                *basePipelineObj->getPipeline()
            );

            auto vertexBindingDescriptions   = std::vector <VkVertexInputBindingDescription> {};
            auto vertexAttributeDescriptions = std::vector <VkVertexInputAttributeDescription> {};
            auto colorBlendAttachments       = std::vector <VkPipelineColorBlendAttachmentState> {};
            auto colorBlendConstants         = std::vector <float> {};
            auto dynamicStates               = std::vector <VkDynamicState> {};
            auto viewPorts                   = std::vector <VkViewport> {};
            auto scissors                    = std::vector <VkRect2D> {};
            auto perFrameBindingFlags        = std::vector <VkDescriptorBindingFlags> {};
            auto perFrameLayoutBindings      = std::vector <VkDescriptorSetLayoutBinding> {};
            auto otherBindingFlags           = std::vector <VkDescriptorBindingFlags> {};
            auto otherLayoutBindings         = std::vector <VkDescriptorSetLayoutBinding> {};

            {   /* Vertex input */
                vertexBindingDescriptions   = {
                    pipelineObj->createVertexBindingDescription (
                        0,
                        sizeof (Vertex),
                        VK_VERTEX_INPUT_RATE_VERTEX
                    )
                };
                vertexAttributeDescriptions = {
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
            }
            {   /* Input assembly */
                pipelineObj->createInputAssemblyState (
                    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                    VK_FALSE
                );
            }
            {   /* Shader stage */
                pipelineObj->addShaderStage (
                    VK_SHADER_STAGE_VERTEX_BIT,
                    "Build/Bin/FDefault[VERT].spv",
                    "main"
                );
                pipelineObj->addShaderStage (
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                    "Build/Bin/FDefault[FRAG].spv",
                    "main"
                );
            }
            {   /* Depth stencil */
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
            }
            {   /* Rasterization */
                pipelineObj->createRasterizationState (
                    1.0f,
                    VK_POLYGON_MODE_FILL,
                    VK_CULL_MODE_NONE,
                    VK_FRONT_FACE_COUNTER_CLOCKWISE,
                    VK_FALSE,
                    0.0f,
                    0.0f
                );
            }
            {   /* Multi sample */
                pipelineObj->createMultiSampleState (
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_FALSE,
                    0.0f
                );
            }
            {   /* Color blend */
                /* The most common way to use color blending is to implement alpha blending, where we want the new color
                 * to be blended with the old color based on its opacity
                 * finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor
                 * finalColor.a   = newAlpha.a
                */
                colorBlendAttachments = {
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
                colorBlendConstants   = {
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
            }
            {   /* Dynamic state */
                dynamicStates = {
                    VK_DYNAMIC_STATE_VIEWPORT,
                    VK_DYNAMIC_STATE_SCISSOR
                };
                pipelineObj->createDynamicState (dynamicStates);
            }
            {   /* View port */
                viewPorts = {};
                scissors  = {};
                pipelineObj->createViewPortState (
                    viewPorts,
                    scissors,
                    1,
                    1
                );
            }
            {   /* Descriptor set layout */
                perFrameBindingFlags   = {
                    0
                };
                perFrameLayoutBindings = {
                    pipelineObj->createDescriptorSetLayoutBinding (
                        0,
                        1,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        VK_SHADER_STAGE_VERTEX_BIT
                    )
                };
                otherBindingFlags      = {
                    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT
                };
                otherLayoutBindings    = {
                    pipelineObj->createDescriptorSetLayoutBinding (
                        0,
                        static_cast <uint32_t> (stdTexturePool.size()),
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
                    otherBindingFlags,
                    otherLayoutBindings
                );
            }
            {   /* Push constant range */
                pipelineObj->addPushConstantRange (
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    sizeof (ActiveCameraPC)
                );
            }

            collectionObj->addCollectionTypeInstance <Renderer::VKPipeline> ("F_DEFAULT", pipelineObj);
            pipelineObj->destroyShaderModules();
        }
        {   /* Descriptor pool  [F_DEFAULT] */
            auto descPoolObj = new Renderer::VKDescriptorPool (logObj, logDeviceObj);
            descPoolObj->initDescriptorPoolInfo (
                VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT,
                g_maxFramesInFlight + 1
            );
            descPoolObj->addDescriptorPoolSize (
                g_maxFramesInFlight,                                /* F_DEFAULT_MESH_INSTANCE_? */
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
            );
            descPoolObj->addDescriptorPoolSize (
                static_cast <uint32_t> (stdTexturePool.size()),     /* G_DEFAULT_TEXTURE_?       */
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorPool> ("F_DEFAULT", descPoolObj);
        }
        {   /* Descriptor sets  [F_DEFAULT_PER_FRAME] */
            auto pipelineObj = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>       ("F_DEFAULT");
            auto descPoolObj = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorPool> ("F_DEFAULT");
            auto descSetObj  = new Renderer::VKDescriptorSet (logObj, logDeviceObj, descPoolObj);
            descSetObj->initDescriptorSetInfo (
                g_maxFramesInFlight,
                pipelineObj->getDescriptorSetLayouts()[0]
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorSet> ("F_DEFAULT_PER_FRAME", descSetObj);

            typedef std::vector        <std::vector <VkDescriptorBufferInfo>> descriptorBufferInfosPool;
            auto descriptorImageInfos = std::vector <VkDescriptorImageInfo> {};
            std::unordered_map <uint32_t, descriptorBufferInfosPool> bindingToBufferInfosMap;

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                {   /* Binding 0 */
                    auto bufferObj             = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                        "F_DEFAULT_MESH_INSTANCE_" + std::to_string (i)
                    );
                    auto descriptorBufferInfos = std::vector <VkDescriptorBufferInfo> {
                        descSetObj->createDescriptorBufferInfo (
                            *bufferObj->getBuffer(),
                            0,
                            bufferObj->getBufferSize()
                        )
                    };
                    bindingToBufferInfosMap[0].push_back (
                        descriptorBufferInfos
                    );
                }
            }
            /* Write and update descriptor sets */
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                {   /* Binding 0 */
                    descSetObj->addWriteDescriptorSet (
                        0,
                        static_cast <uint32_t> (bindingToBufferInfosMap[0][i].size()),
                        0,
                        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        descSetObj->getDescriptorSets()[i],
                        bindingToBufferInfosMap[0][i],
                        descriptorImageInfos
                    );
                }
            }
            descSetObj->updateDescriptorSets();
        }
        {   /* Descriptor sets  [F_DEFAULT_OTHER] */
            auto pipelineObj = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>       ("F_DEFAULT");
            auto descPoolObj = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorPool> ("F_DEFAULT");
            auto descSetObj  = new Renderer::VKDescriptorSet (logObj, logDeviceObj, descPoolObj);
            descSetObj->initDescriptorSetInfo (
                1,
                pipelineObj->getDescriptorSetLayouts()[1]
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorSet> ("F_DEFAULT_OTHER", descSetObj);

            auto descriptorBufferInfos =  std::vector <VkDescriptorBufferInfo> {};
            std::unordered_map <uint32_t, std::vector <VkDescriptorImageInfo>> bindingToImageInfosMap;

            {   /* Binding 0 */
                for (auto const& [idx, info]: stdTexturePool) {
                    auto imageObj   = collectionObj->getCollectionTypeInstance <Renderer::VKImage>   (
                        "G_DEFAULT_TEXTURE_" + std::to_string (idx)
                    );
                    auto samplerObj = collectionObj->getCollectionTypeInstance <Renderer::VKSampler> (
                        "G_DEFAULT_TEXTURE"
                    );
                    bindingToImageInfosMap[0].push_back (
                        descSetObj->createDescriptorImageInfo (
                            *samplerObj->getSampler(),
                            *imageObj->getImageView(),
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                        )
                    );
                }
            }
            /* Write and update descriptor sets */
            {   /* Binding 0 */
                descSetObj->addWriteDescriptorSet (
                    0,
                    static_cast <uint32_t> (bindingToImageInfosMap[0].size()),
                    0,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    descSetObj->getDescriptorSets()[0],
                    descriptorBufferInfos,
                    bindingToImageInfosMap[0]
                );
            }
            descSetObj->updateDescriptorSets();
        }

        {   /* Pipeline         [F_DEBUG] */
            auto renderPassObj   = collectionObj->getCollectionTypeInstance <Renderer::VKRenderPass> ("F");
            auto basePipelineObj = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>   ("F_LIGHT");
            auto pipelineObj     = new Renderer::VKPipeline (logObj, logDeviceObj, renderPassObj);
            pipelineObj->initPipelineInfo (
                 VK_PIPELINE_CREATE_DERIVATIVE_BIT,
                 0,
                -1,
                *basePipelineObj->getPipeline()
            );

            auto vertexBindingDescriptions   = std::vector <VkVertexInputBindingDescription> {};
            auto vertexAttributeDescriptions = std::vector <VkVertexInputAttributeDescription> {};
            auto colorBlendAttachments       = std::vector <VkPipelineColorBlendAttachmentState> {};
            auto colorBlendConstants         = std::vector <float> {};
            auto dynamicStates               = std::vector <VkDynamicState> {};
            auto viewPorts                   = std::vector <VkViewport> {};
            auto scissors                    = std::vector <VkRect2D> {};
            auto otherBindingFlags           = std::vector <VkDescriptorBindingFlags> {};
            auto otherLayoutBindings         = std::vector <VkDescriptorSetLayoutBinding> {};

            {   /* Vertex input */
                vertexBindingDescriptions   = {};
                vertexAttributeDescriptions = {};
                pipelineObj->createVertexInputState (
                    vertexBindingDescriptions,
                    vertexAttributeDescriptions
                );
            }
            {   /* Input assembly */
                pipelineObj->createInputAssemblyState (
                    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                    VK_FALSE
                );
            }
            {   /* Shader stage */
                pipelineObj->addShaderStage (
                    VK_SHADER_STAGE_VERTEX_BIT,
                    "Build/Bin/Debug[VERT].spv",
                    "main"
                );
                pipelineObj->addShaderStage (
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                    "Build/Bin/Debug[FRAG].spv",
                    "main"
                );
            }
            {   /* Depth stencil */
                pipelineObj->createDepthStencilState (
                    VK_FALSE,
                    VK_FALSE,
                    VK_FALSE,
                    VK_FALSE,
                    VK_COMPARE_OP_NEVER,
                    0.0f,
                    1.0f,
                    {},
                    {}
                );
            }
            {   /* Rasterization */
                pipelineObj->createRasterizationState (
                    1.0f,
                    VK_POLYGON_MODE_FILL,
                    VK_CULL_MODE_BACK_BIT,
                    VK_FRONT_FACE_COUNTER_CLOCKWISE,
                    VK_FALSE,
                    0.0f,
                    0.0f
                );
            }
            {   /* Multi sample */
                pipelineObj->createMultiSampleState (
                    VK_SAMPLE_COUNT_1_BIT,
                    VK_FALSE,
                    0.0f
                );
            }
            {   /* Color blend */
                colorBlendAttachments = {
                    pipelineObj->createColorBlendAttachment (
                        VK_FALSE,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_BLEND_FACTOR_ONE,
                        VK_BLEND_FACTOR_ZERO,
                        VK_BLEND_OP_ADD,
                        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
                    )
                };
                colorBlendConstants   = {
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
            }
            {   /* Dynamic state */
                dynamicStates = {
                    VK_DYNAMIC_STATE_VIEWPORT,
                    VK_DYNAMIC_STATE_SCISSOR
                };
                pipelineObj->createDynamicState (dynamicStates);
            }
            {   /* View port */
                viewPorts = {};
                scissors  = {};
                pipelineObj->createViewPortState (
                    viewPorts,
                    scissors,
                    1,
                    1
                );
            }
            {   /* Descriptor set layout */
                otherBindingFlags   = {
                    0
                };
                otherLayoutBindings = {
                    pipelineObj->createDescriptorSetLayoutBinding (
                        0,
                        8,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT
                    )
                };
                /* Layout 0 */
                pipelineObj->addDescriptorSetLayout (
                    0,
                    otherBindingFlags,
                    otherLayoutBindings
                );
            }

            collectionObj->addCollectionTypeInstance <Renderer::VKPipeline> ("F_DEBUG", pipelineObj);
            pipelineObj->destroyShaderModules();
        }
        {   /* Descriptor pool  [F_DEBUG] */
            auto descPoolObj = new Renderer::VKDescriptorPool (logObj, logDeviceObj);
            descPoolObj->initDescriptorPoolInfo (
                0,
                1
            );
            descPoolObj->addDescriptorPoolSize (
                8,                                              /* Debug slots */
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorPool> ("F_DEBUG", descPoolObj);
        }
        {   /* Descriptor sets  [F_DEBUG_OTHER] */
            auto pipelineObj = collectionObj->getCollectionTypeInstance <Renderer::VKPipeline>       ("F_DEBUG");
            auto descPoolObj = collectionObj->getCollectionTypeInstance <Renderer::VKDescriptorPool> ("F_DEBUG");
            auto descSetObj  = new Renderer::VKDescriptorSet (logObj, logDeviceObj, descPoolObj);
            descSetObj->initDescriptorSetInfo (
                1,
                pipelineObj->getDescriptorSetLayouts()[0]
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKDescriptorSet> ("F_DEBUG_OTHER", descSetObj);

            auto descriptorBufferInfos =  std::vector <VkDescriptorBufferInfo> {};
            std::unordered_map <uint32_t, std::vector <VkDescriptorImageInfo>> bindingToImageInfosMap;

            auto instanceIds = std::vector <std::string> {
                "G_DEFAULT_NORMAL",
                "G_DEFAULT_POSITION",
                "G_DEFAULT_COLOR_0",
                "G_DEFAULT_COLOR_1",
                "S_DEFAULT_DEPTH_0",
                "S_DEFAULT_DEPTH_1",
                "S_DEFAULT_DEPTH_2",
                "S_CUBE_DEPTH_0"
            };
            {   /* Binding 0 */
                for (auto const& instanceId: instanceIds) {
                    auto imageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage> (instanceId);

                    bindingToImageInfosMap[0].push_back (
                        descSetObj->createDescriptorImageInfo (
                            *gBufferSamplerObj->getSampler(),
                            *imageObj->getImageView(),
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                        )
                    );
                }
            }
            /* Write and update descriptor sets */
            {   /* Binding 0 */
                descSetObj->addWriteDescriptorSet (
                    0,
                    static_cast <uint32_t> (bindingToImageInfosMap[0].size()),
                    0,
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    descSetObj->getDescriptorSets()[0],
                    descriptorBufferInfos,
                    bindingToImageInfosMap[0]
                );
            }
            descSetObj->updateDescriptorSets();
        }
    }

    void SBImpl::configRendererOps (void) {
        auto& collectionObj        = m_sandBoxInfo.resource.collectionObj;
        auto& stdTexturePoolObj    = m_sandBoxInfo.resource.stdTexturePoolObj;
        auto& skyBoxTexturePoolObj = m_sandBoxInfo.resource.skyBoxTexturePoolObj;

        auto logObj                = collectionObj->getCollectionTypeInstance <Log::LGImpl>           ("CORE");
        auto windowObj             = collectionObj->getCollectionTypeInstance <Renderer::VKWindow>    ("CORE");
        auto phyDeviceObj          = collectionObj->getCollectionTypeInstance <Renderer::VKPhyDevice> ("CORE");
        auto logDeviceObj          = collectionObj->getCollectionTypeInstance <Renderer::VKLogDevice> ("CORE");
        auto swapChainObj          = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain> ("CORE");
        auto stdTexturePool        = stdTexturePoolObj->getTexturePool();
        auto skyBoxTexturePool     = skyBoxTexturePoolObj->getTexturePool();

        {   /* Fence            [COPY_OPS] */
            auto fenObj = new Renderer::VKFence (logObj, logDeviceObj);
            fenObj->initFenceInfo (
                0
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKFence> ("COPY_OPS", fenObj);
        }
        {   /* Cmd pool         [COPY_OPS] */
            auto cmdPoolObj = new Renderer::VKCmdPool (logObj, logDeviceObj);
            /* Note that the command buffer that we will be allocating from this pool will be short lived, hence why
             * we will choose the VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag
            */
            cmdPoolObj->initCmdPoolInfo (
                VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                phyDeviceObj->getTransferQueueFamilyIdx()
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKCmdPool> ("COPY_OPS", cmdPoolObj);
        }
        {   /* Cmd buffers      [COPY_OPS] */
            auto fenObj     = collectionObj->getCollectionTypeInstance <Renderer::VKFence>   ("COPY_OPS");
            auto cmdPoolObj = collectionObj->getCollectionTypeInstance <Renderer::VKCmdPool> ("COPY_OPS");
            auto bufferObj  = new Renderer::VKCmdBuffer (logObj, logDeviceObj, cmdPoolObj);
            /* Note that we are only requesting one command buffer from the pool, since it is recommended to combine
             * operations in a single command buffer and execute them asynchronously for higher throughput
            */
            bufferObj->initCmdBufferInfo (
                1,
                VK_COMMAND_BUFFER_LEVEL_PRIMARY
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKCmdBuffer> ("COPY_OPS", bufferObj);
            /* We're only going to record the command buffer once and wait until the operations have finished executing.
             * It's good practice to tell the driver about our intent using VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
            */
            Renderer::beginCmdBufferRecording (
                bufferObj->getCmdBuffers()[0],
                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
            );
            {   /* Vertex buffer->buffer        [S_DEFAULT] */
                auto srcBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "S_DEFAULT_VERTEX_STAGING"
                );
                auto dstBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "S_DEFAULT_VERTEX"
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
            {   /* Index  buffer->buffer        [S_DEFAULT] */
                auto srcBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "S_DEFAULT_INDEX_STAGING"
                );
                auto dstBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "S_DEFAULT_INDEX"
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

            {   /* Buffer->Image                [G_DEFAULT] */
                for (auto const& [idx, info]: stdTexturePool) {
                    auto srcBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                        "G_DEFAULT_TEXTURE_STAGING_" + std::to_string (idx)
                    );
                    auto dstImageObj  = collectionObj->getCollectionTypeInstance <Renderer::VKImage>  (
                        "G_DEFAULT_TEXTURE_"         + std::to_string (idx)
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
                            dstImageObj->getImageExtent().width,
                            dstImageObj->getImageExtent().height,
                            1
                        },
                        dstImageObj->getImageMipLevels(),
                        0,
                        dstImageObj->getImageLayersCount(),
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        dstImageObj->getImageAspectFlags(),
                        copyRegions
                    );
                }
            }

            {   /* Vertex buffer->buffer        [F_WIRE] */
                auto srcBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "F_WIRE_VERTEX_STAGING"
                );
                auto dstBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "F_WIRE_VERTEX"
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
            {   /* Index  buffer->buffer        [F_WIRE] */
                auto srcBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "F_WIRE_INDEX_STAGING"
                );
                auto dstBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "F_WIRE_INDEX"
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

            {   /* Vertex buffer->buffer        [F_SKY_BOX] */
                auto srcBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "F_SKY_BOX_VERTEX_STAGING"
                );
                auto dstBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "F_SKY_BOX_VERTEX"
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
            {   /* Index  buffer->buffer        [F_SKY_BOX] */
                auto srcBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "F_SKY_BOX_INDEX_STAGING"
                );
                auto dstBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "F_SKY_BOX_INDEX"
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
            {   /* Buffer->Image                [F_SKY_BOX] */
                for (auto const& [idx, info]: skyBoxTexturePool) {
                    auto srcBufferObj   = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                        "F_SKY_BOX_TEXTURE_STAGING_" + std::to_string (idx)
                    );
                    auto dstImageObj    = collectionObj->getCollectionTypeInstance <Renderer::VKImage>  (
                        "F_SKY_BOX_TEXTURE"
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
                            dstImageObj->getImageExtent().width,
                            dstImageObj->getImageExtent().height,
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

            {   /* Vertex buffer->buffer        [F_DEFAULT] */
                auto srcBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "F_DEFAULT_VERTEX_STAGING"
                );
                auto dstBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "F_DEFAULT_VERTEX"
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
            {   /* Index  buffer->buffer        [F_DEFAULT] */
                auto srcBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "F_DEFAULT_INDEX_STAGING"
                );
                auto dstBufferObj = collectionObj->getCollectionTypeInstance <Renderer::VKBuffer> (
                    "F_DEFAULT_INDEX"
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
            Renderer::endCmdBufferRecording (
                bufferObj->getCmdBuffers()[0]
            );

            auto cmdBuffers       = std::vector {
                bufferObj->getCmdBuffers()[0]
            };
            auto waitSemaphores   = std::vector <VkSemaphore> {};
            auto waitStageMasks   = std::vector <VkPipelineStageFlags> {};
            auto signalSemaphores = std::vector <VkSemaphore> {};
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
            collectionObj->removeCollectionTypeInstance <Renderer::VKCmdBuffer>  ("COPY_OPS");
            collectionObj->removeCollectionTypeInstance <Renderer::VKCmdPool>    ("COPY_OPS");
            collectionObj->removeCollectionTypeInstance <Renderer::VKFence>      ("COPY_OPS");
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>     ("F_DEFAULT_INDEX_STAGING");
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>     ("F_DEFAULT_VERTEX_STAGING");
            for (auto const& [idx, info]: skyBoxTexturePool)
                collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer> (
                    "F_SKY_BOX_TEXTURE_STAGING_" + std::to_string (idx)
                );
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>     ("F_SKY_BOX_INDEX_STAGING");
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>     ("F_SKY_BOX_VERTEX_STAGING");
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>     ("F_WIRE_INDEX_STAGING");
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>     ("F_WIRE_VERTEX_STAGING");
            for (auto const& [idx, info]: stdTexturePool)
                collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer> (
                    "G_DEFAULT_TEXTURE_STAGING_" + std::to_string (idx)
                );
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>     ("S_DEFAULT_INDEX_STAGING");
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>     ("S_DEFAULT_VERTEX_STAGING");
        }

        {   /* Fence            [BLIT_OPS] */
            auto fenObj = new Renderer::VKFence (logObj, logDeviceObj);
            fenObj->initFenceInfo (
                0
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKFence> ("BLIT_OPS", fenObj);
        }
        {   /* Cmd pool         [BLIT_OPS] */
            auto cmdPoolObj = new Renderer::VKCmdPool (logObj, logDeviceObj);
            cmdPoolObj->initCmdPoolInfo (
                VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                phyDeviceObj->getGraphicsQueueFamilyIdx()
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKCmdPool> ("BLIT_OPS", cmdPoolObj);
        }
        {   /* Cmd buffers      [BLIT_OPS] */
            auto fenObj     = collectionObj->getCollectionTypeInstance <Renderer::VKFence>   ("BLIT_OPS");
            auto cmdPoolObj = collectionObj->getCollectionTypeInstance <Renderer::VKCmdPool> ("BLIT_OPS");
            auto bufferObj  = new Renderer::VKCmdBuffer (logObj, logDeviceObj, cmdPoolObj);
            bufferObj->initCmdBufferInfo (
                1,
                VK_COMMAND_BUFFER_LEVEL_PRIMARY
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKCmdBuffer> ("BLIT_OPS", bufferObj);
            Renderer::beginCmdBufferRecording (
                bufferObj->getCmdBuffers()[0],
                VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
            );
            {   /* Image->Image                 [G_DEFAULT] */
                for (auto const& [idx, info]: stdTexturePool) {
                    auto imageObj = collectionObj->getCollectionTypeInstance <Renderer::VKImage> (
                        "G_DEFAULT_TEXTURE_" + std::to_string (idx)
                    );
                    Renderer::blitImageToMipMaps (
                        bufferObj->getCmdBuffers()[0],
                        *imageObj->getImage(),
                        imageObj->getImageExtent().width,
                        imageObj->getImageExtent().height,
                        imageObj->getImageMipLevels(),
                        0,
                        imageObj->getImageAspectFlags()
                    );
                }
            }
            Renderer::endCmdBufferRecording (
                bufferObj->getCmdBuffers()[0]
            );

            auto cmdBuffers       = std::vector {
                bufferObj->getCmdBuffers()[0]
            };
            auto waitSemaphores   = std::vector <VkSemaphore> {};
            auto waitStageMasks   = std::vector <VkPipelineStageFlags> {};
            auto signalSemaphores = std::vector <VkSemaphore> {};
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
            collectionObj->removeCollectionTypeInstance <Renderer::VKCmdBuffer> ("BLIT_OPS");
            collectionObj->removeCollectionTypeInstance <Renderer::VKCmdPool>   ("BLIT_OPS");
            collectionObj->removeCollectionTypeInstance <Renderer::VKFence>     ("BLIT_OPS");
        }

        {   /* Fence            [IN_FLIGHT_?] */
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
                    "IN_FLIGHT_" + std::to_string (i),
                    fenObj
                );
            }
        }
        {   /* Semaphore        [IMG_AVAILABLE_?] */
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                auto semObj = new Renderer::VKSemaphore (logObj, logDeviceObj);
                semObj->initSemaphoreInfo (
                    0
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKSemaphore> (
                    "IMG_AVAILABLE_" + std::to_string (i),
                    semObj
                );
            }
        }
        {   /* Semaphore        [RENDER_DONE_?] */
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                auto semObj = new Renderer::VKSemaphore (logObj, logDeviceObj);
                semObj->initSemaphoreInfo (
                    0
                );

                collectionObj->addCollectionTypeInstance <Renderer::VKSemaphore> (
                    "RENDER_DONE_" + std::to_string (i),
                    semObj
                );
            }
        }
        {   /* Cmd pool         [DRAW_OPS] */
            auto cmdPoolObj = new Renderer::VKCmdPool (logObj, logDeviceObj);
            /* We will be recording a command buffer every frame, so we want to be able to reset and rerecord over
             * it, hence why we need the VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT flag
            */
            cmdPoolObj->initCmdPoolInfo (
                VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                phyDeviceObj->getGraphicsQueueFamilyIdx()
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKCmdPool> ("DRAW_OPS", cmdPoolObj);
        }
        {   /* Cmd buffers      [DRAW_OPS] */
            auto cmdPoolObj = collectionObj->getCollectionTypeInstance <Renderer::VKCmdPool> ("DRAW_OPS");
            auto bufferObj  = new Renderer::VKCmdBuffer (logObj, logDeviceObj, cmdPoolObj);
            bufferObj->initCmdBufferInfo (
                g_maxFramesInFlight,
                VK_COMMAND_BUFFER_LEVEL_PRIMARY
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKCmdBuffer> ("DRAW_OPS", bufferObj);
        }
        {   /* Renderer         [DRAW_OPS] */
            auto inFlightFenObjs       = std::vector <Renderer::VKFence*>     {};
            auto imageAvailableSemObjs = std::vector <Renderer::VKSemaphore*> {};
            auto renderDoneSemObjs     = std::vector <Renderer::VKSemaphore*> {};
            auto bufferObj             = collectionObj->getCollectionTypeInstance <Renderer::VKCmdBuffer> ("DRAW_OPS");

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++) {
                auto inFlightFenObj       = collectionObj->getCollectionTypeInstance <Renderer::VKFence>     (
                    "IN_FLIGHT_"     + std::to_string (i)
                );
                auto imageAvailableSemObj = collectionObj->getCollectionTypeInstance <Renderer::VKSemaphore> (
                    "IMG_AVAILABLE_" + std::to_string (i)
                );
                auto renderDoneSemObj     = collectionObj->getCollectionTypeInstance <Renderer::VKSemaphore> (
                    "RENDER_DONE_"   + std::to_string (i)
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
                bufferObj
            );
            rendererObj->initRendererInfo (
                g_maxFramesInFlight
            );

            collectionObj->addCollectionTypeInstance <Renderer::VKRenderer> ("DRAW_OPS", rendererObj);
            /* Add view port resize bindings */
            rendererObj->addViewPortResizeBinding (
                [](void) {}
            );
        }
    }

    void SBImpl::destroyRenderer (void) {
        auto& sceneObj          = m_sandBoxInfo.resource.sceneObj;
        auto& collectionObj     = m_sandBoxInfo.resource.collectionObj;
        auto& stdTexturePoolObj = m_sandBoxInfo.resource.stdTexturePoolObj;

        auto swapChainObj       = collectionObj->getCollectionTypeInstance <Renderer::VKSwapChain> ("CORE");
        auto stdTexturePool     = stdTexturePoolObj->getTexturePool();

        {   /* Ops */
            collectionObj->removeCollectionTypeInstance <Renderer::VKRenderer>      ("DRAW_OPS");
            collectionObj->removeCollectionTypeInstance <Renderer::VKCmdBuffer>     ("DRAW_OPS");
            collectionObj->removeCollectionTypeInstance <Renderer::VKCmdPool>       ("DRAW_OPS");

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKSemaphore> (
                    "RENDER_DONE_"   + std::to_string (i)
                );
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKSemaphore> (
                    "IMG_AVAILABLE_" + std::to_string (i)
                );
            for (uint32_t i = 0; i < g_maxFramesInFlight; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKFence>     (
                    "IN_FLIGHT_"     + std::to_string (i)
                );
        }
        {   /* F pass */
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorSet>   ("F_DEBUG_OTHER");
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorPool>  ("F_DEBUG");
            collectionObj->removeCollectionTypeInstance <Renderer::VKPipeline>        ("F_DEBUG");

            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorSet>   ("F_DEFAULT_OTHER");
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorSet>   ("F_DEFAULT_PER_FRAME");
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorPool>  ("F_DEFAULT");
            collectionObj->removeCollectionTypeInstance <Renderer::VKPipeline>        ("F_DEFAULT");

            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorSet>   ("F_SKY_BOX_OTHER");
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorSet>   ("F_SKY_BOX_PER_FRAME");
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorPool>  ("F_SKY_BOX");
            collectionObj->removeCollectionTypeInstance <Renderer::VKPipeline>        ("F_SKY_BOX");

            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorSet>   ("F_WIRE_PER_FRAME");
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorPool>  ("F_WIRE");
            collectionObj->removeCollectionTypeInstance <Renderer::VKPipeline>        ("F_WIRE");

            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorSet>   ("F_LIGHT_OTHER");
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorSet>   ("F_LIGHT_PER_FRAME");
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorPool>  ("F_LIGHT");
            collectionObj->removeCollectionTypeInstance <Renderer::VKPipeline>        ("F_LIGHT");

            for (uint32_t i = 0; i < swapChainObj->getSwapChainImagesCount(); i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKFrameBuffer> (
                    "F_"                       + std::to_string (i)
                );
            collectionObj->removeCollectionTypeInstance <Renderer::VKRenderPass>      ("F");

            collectionObj->removeCollectionTypeInstance <Renderer::VKSampler>         ("F_SKY_BOX_TEXTURE");
            collectionObj->removeCollectionTypeInstance <Renderer::VKSampler>         ("F_LIGHT_GBUFFER");
            collectionObj->removeCollectionTypeInstance <Renderer::VKImage>           ("F_SKY_BOX_TEXTURE");

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>      (
                    "F_DEFAULT_MESH_INSTANCE_" + std::to_string (i)
                );
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>          ("F_DEFAULT_INDEX");
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>          ("F_DEFAULT_VERTEX");

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>      (
                    "F_SKY_BOX_MESH_INSTANCE_" + std::to_string (i)
                );
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>          ("F_SKY_BOX_INDEX");
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>          ("F_SKY_BOX_VERTEX");

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>      (
                    "F_WIRE_MESH_INSTANCE_"    + std::to_string (i)
                );
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>          ("F_WIRE_INDEX");
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>          ("F_WIRE_VERTEX");

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>      (
                    "F_LIGHT_LIGHT_INSTANCE_"  + std::to_string (i)
                );
        }
        {   /* G pass */
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorSet>  ("G_DEFAULT_OTHER");
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorSet>  ("G_DEFAULT_PER_FRAME");
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorPool> ("G_DEFAULT");
            collectionObj->removeCollectionTypeInstance <Renderer::VKPipeline>       ("G_DEFAULT");

            collectionObj->removeCollectionTypeInstance <Renderer::VKFrameBuffer>    ("G");
            collectionObj->removeCollectionTypeInstance <Renderer::VKRenderPass>     ("G");

            collectionObj->removeCollectionTypeInstance <Renderer::VKSampler>        ("G_DEFAULT_TEXTURE");
            for (auto const& [idx, info]: stdTexturePool)
                collectionObj->removeCollectionTypeInstance <Renderer::VKImage>      (
                    "G_DEFAULT_TEXTURE_"       + std::to_string (idx)
                );
            collectionObj->removeCollectionTypeInstance <Renderer::VKImage>          ("G_DEFAULT_DEPTH");
            collectionObj->removeCollectionTypeInstance <Renderer::VKImage>          ("G_DEFAULT_COLOR_2");
            collectionObj->removeCollectionTypeInstance <Renderer::VKImage>          ("G_DEFAULT_COLOR_1");
            collectionObj->removeCollectionTypeInstance <Renderer::VKImage>          ("G_DEFAULT_COLOR_0");
            collectionObj->removeCollectionTypeInstance <Renderer::VKImage>          ("G_DEFAULT_POSITION");
            collectionObj->removeCollectionTypeInstance <Renderer::VKImage>          ("G_DEFAULT_NORMAL");

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>     (
                    "G_DEFAULT_MESH_INSTANCE_" + std::to_string (i)
                );
        }
        {   /* S pass */
            auto batchingObj          = sceneObj->getSystem <SYLightInstanceBatching>();
            auto lightTypeOffsets     = batchingObj->getLightTypeOffsets();
            uint32_t otherLightsCount = lightTypeOffsets->pointLightsOffset;
            uint32_t pointLightsCount = lightTypeOffsets->lightsCount - lightTypeOffsets->pointLightsOffset;
            size_t activeLightIdx     = 0;

            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorSet>   ("S_CUBE_PER_FRAME");
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorPool>  ("S_CUBE");
            collectionObj->removeCollectionTypeInstance <Renderer::VKPipeline>        ("S_CUBE");

            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorSet>   ("S_DEFAULT_PER_FRAME");
            collectionObj->removeCollectionTypeInstance <Renderer::VKDescriptorPool>  ("S_DEFAULT");
            collectionObj->removeCollectionTypeInstance <Renderer::VKPipeline>        ("S_DEFAULT");

            for (uint32_t i = 0; i < otherLightsCount; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKFrameBuffer> (
                    "S_"                       + std::to_string (activeLightIdx++)
                );
            for (uint32_t i = 0; i < pointLightsCount; i++) {
            for (uint32_t cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKFrameBuffer> (
                    "S_"                       + std::to_string (activeLightIdx++)
                );
            }
            collectionObj->removeCollectionTypeInstance <Renderer::VKRenderPass>      ("S");

            for (uint32_t i = 0; i < pointLightsCount; i++) {
            for (uint32_t cubeFaceIdx = 0; cubeFaceIdx < 6; cubeFaceIdx++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKImage>       (
                    "S_CUBE_DEPTH_"            + std::to_string ((i * 6) + cubeFaceIdx)
                );
            }
            for (uint32_t i = 0; i < pointLightsCount; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKImage>       (
                    "F_LIGHT_DEPTH_CUBE_"      + std::to_string (i)
                );
            for (uint32_t i = 0; i < otherLightsCount; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKImage>       (
                    "S_DEFAULT_DEPTH_"         + std::to_string (i)
                );

            for (uint32_t i = 0; i < g_maxFramesInFlight; i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>      (
                    "S_DEFAULT_MESH_INSTANCE_" + std::to_string (i)
                );
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>          ("S_DEFAULT_INDEX");
            collectionObj->removeCollectionTypeInstance <Renderer::VKBuffer>          ("S_DEFAULT_VERTEX");
        }
        {   /* Core */
            for (uint32_t i = 0; i < swapChainObj->getSwapChainImagesCount(); i++)
                collectionObj->removeCollectionTypeInstance <Renderer::VKImage> (
                    "SWAP_CHAIN_" + std::to_string (i)
                );

            collectionObj->removeCollectionTypeInstance <Renderer::VKSwapChain> ("CORE");
            collectionObj->removeCollectionTypeInstance <Renderer::VKLogDevice> ("CORE");
            collectionObj->removeCollectionTypeInstance <Renderer::VKPhyDevice> ("CORE");
            collectionObj->removeCollectionTypeInstance <Renderer::VKSurface>   ("CORE");
            collectionObj->removeCollectionTypeInstance <Renderer::VKWindow>    ("CORE");
            collectionObj->removeCollectionTypeInstance <Renderer::VKInstance>  ("CORE");
            collectionObj->removeCollectionTypeInstance <Log::LGImpl>           ("CORE");
        }
    }
}   // namespace SandBox