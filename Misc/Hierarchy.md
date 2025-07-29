# Class/file hierarchy

## Backend/
<pre>
    |<----------------------:glfw3
    |<----------------------:vk_enum_string_helper
    |<----------------------:glm
    |<----------------------:hash
    |<----------------------:matrix_transform
    |<----------------------:quaternion
    |<----------------------:set
    |<----------------------:map
    |<----------------------:vector
    |<----------------------:array
    |<----------------------:queue
    |<----------------------:string
    |<----------------------:bitset
    |<----------------------:utility
    |<----------------------:optional
    |<----------------------:unordered_map
    |<----------------------:fstream
    |<----------------------:sstream
    |<----------------------:iostream
    |<----------------------:cmath
    |<----------------------:limits
    |<----------------------:algorithm
    |<----------------------:chrono
    |<----------------------:cstdint
    |<----------------------:iomanip
    |<----------------------:iterator
    |<----------------------:stdexcept
    |<----------------------:functional
    |Common
</pre>

## Backend/Scene/
<pre>
    |<----------------------:Common
    |<----------------------:LGImpl
    |<----------------------:SNType
    |SNEntityMgr

    |<----------------------:SNType
    |SNComponentArrayBase

    |<----------------------:Common
    |<----------------------|SNComponentArrayBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:SNType
    |SNComponentArray

    |<----------------------:Common
    |<----------------------:SNComponentArrayBase
    |<----------------------:SNComponentArray
    |<----------------------:LGImpl
    |<----------------------:SNType
    |SNComponentMgr

    |<----------------------:Common
    |<----------------------:SNType
    |SNSystemBase

    |<----------------------:Common
    |<----------------------:SNSystemBase
    |<----------------------:LGImpl
    |<----------------------:SNType
    |SNSystemMgr

    |<----------------------:Common
    |<----------------------:SNEntityMgr
    |<----------------------:SNComponentMgr
    |<----------------------:SNSystemMgr
    |<----------------------:LGImpl
    |<----------------------:SNType
    |SNImpl

    |<----------------------:Common
    |SNType
</pre>

## Backend/Collection/
<pre>
    |<----------------------:Common
    |<----------------------:CNTypeInstanceBase
    |<----------------------:LGImpl
    |CNTypeInstanceArray

    |<----------------------:Common
    |<----------------------:CNTypeInstanceBase
    |<----------------------:CNTypeInstanceArray
    |<----------------------:LGImpl
    |CNImpl
</pre>

## Backend/Log/
<pre>
    |<----------------------:Common
    |<----------------------|CNTypeInstanceBase [PUB]
    |LGImpl
</pre>

## Backend/Renderer/
<pre>
    |<----------------------:Common
    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |
    \VKInstance
    \VKWindow

    |<----------------------:Common
    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKInstance
    |<----------------------:VKWindow
    |VKSurface

    |<----------------------:Common
    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKInstance
    |<----------------------:VKSurface
    |VKPhyDevice

    |<----------------------:Common
    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKInstance
    |<----------------------:VKPhyDevice
    |VKLogDevice

    |<----------------------:Common
    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKWindow
    |<----------------------:VKSurface
    |<----------------------:VKPhyDevice
    |<----------------------:VKLogDevice
    |<----------------------:VKHelper
    |VKSwapChain

    |<----------------------:Common
    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKPhyDevice
    |<----------------------:VKLogDevice
    |<----------------------:VKHelper
    |
    \VKBuffer
    \VKImage

    |<----------------------:Common
    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKLogDevice
    |
    \VKSampler
    \VKRenderPass
    \VKDescriptorPool
    \VKFence
    \VKSemaphore
    \VKCmdPool

    |<----------------------:Common
    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKLogDevice
    |<----------------------:VKRenderPass
    |
    \VKFrameBuffer
    \VKPipeline

    |<----------------------:Common
    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKLogDevice
    |<----------------------:VKDescriptorPool
    |VKDescriptorSet

    |<----------------------:Common
    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKLogDevice
    |<----------------------:VKCmdPool
    |VKCmdBuffer

    |<----------------------:Common
    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKWindow
    |<----------------------:VKLogDevice
    |<----------------------:VKSwapChain
    |<----------------------:VKFence
    |<----------------------:VKSemaphore
    |<----------------------:VKCmdBuffer
    |<----------------------:VKHelper
    |VKRenderer

    |<----------------------:Common
    |
    \VKCmdList
    \VKHelper
</pre>

## SandBox/System/
<pre>
    |<----------------------:Common
    |<----------------------|SNSystemBase [PUB]
    |<----------------------:SNImpl
    |<----------------------:LGImpl
    |<----------------------:tiny_obj_loader
    |<----------------------:SBTexturePool
    |<----------------------:SBComponentType
    |<----------------------:SBRendererType
    |SYMeshLoading

    |<----------------------:Common
    |<----------------------|SNSystemBase [PUB]
    |<----------------------:SNImpl
    |<----------------------:LGImpl
    |<----------------------:SNType
    |<----------------------:SBComponentType
    |<----------------------:SBRendererType
    |
    \SYMeshBatching
    \SYLightInstanceBatching

    |<----------------------:Common
    |<----------------------|SNSystemBase [PUB]
    |<----------------------:SNImpl
    |<----------------------:LGImpl
    |<----------------------:SNType
    |<----------------------:SBComponentType
    |
    \SYStdMeshInstanceBatching
    \SYWireMeshInstanceBatching

    |<----------------------:Common
    |<----------------------|SNSystemBase [PUB]
    |<----------------------:SNImpl
    |<----------------------:CNImpl
    |<----------------------:LGImpl
    |<----------------------:VKWindow
    |<----------------------:VKSwapChain
    |<----------------------:SYEnum
    |<----------------------:SBComponentType
    |<----------------------:SBRendererType
    |SYCameraController

    |<----------------------:Common
    |<----------------------|SNSystemBase [PUB]
    |<----------------------:SNImpl
    |<----------------------:CNImpl
    |<----------------------:LGImpl
    |<----------------------:VKBuffer
    |<----------------------:VKImage
    |<----------------------:VKRenderPass
    |<----------------------:VKFrameBuffer
    |<----------------------:VKPipeline
    |<----------------------:VKDescriptorSet
    |<----------------------:VKCmdBuffer
    |<----------------------:VKRenderer
    |<----------------------:VKCmdList
    |<----------------------:SBComponentType
    |<----------------------:SBRendererType
    |
    \SYShadowRendering
    \SYShadowCubeRendering

    |<----------------------:Common
    |<----------------------|SNSystemBase [PUB]
    |<----------------------:SNImpl
    |<----------------------:CNImpl
    |<----------------------:LGImpl
    |<----------------------:VKSwapChain
    |<----------------------:VKBuffer
    |<----------------------:VKRenderPass
    |<----------------------:VKFrameBuffer
    |<----------------------:VKPipeline
    |<----------------------:VKDescriptorSet
    |<----------------------:VKCmdBuffer
    |<----------------------:VKRenderer
    |<----------------------:VKCmdList
    |<----------------------:SBComponentType
    |<----------------------:SBRendererType
    |SYGDefaultRendering

    |<----------------------:Common
    |<----------------------|SNSystemBase [PUB]
    |<----------------------:CNImpl
    |<----------------------:LGImpl
    |<----------------------:VKSwapChain
    |<----------------------:VKBuffer
    |<----------------------:VKRenderPass
    |<----------------------:VKFrameBuffer
    |<----------------------:VKPipeline
    |<----------------------:VKDescriptorSet
    |<----------------------:VKCmdBuffer
    |<----------------------:VKRenderer
    |<----------------------:VKCmdList
    |<----------------------:SBRendererType
    |SYLightRendering

    |<----------------------:Common
    |<----------------------|SNSystemBase [PUB]
    |<----------------------:SNImpl
    |<----------------------:CNImpl
    |<----------------------:LGImpl
    |<----------------------:VKBuffer
    |<----------------------:VKPipeline
    |<----------------------:VKDescriptorSet
    |<----------------------:VKCmdBuffer
    |<----------------------:VKRenderer
    |<----------------------:VKCmdList
    |<----------------------:SBComponentType
    |<----------------------:SBRendererType
    |
    \SYWireRendering
    \SYSkyBoxRendering
    \SYFDefaultRendering

    |<----------------------:Common
    |<----------------------|SNSystemBase [PUB]
    |<----------------------:CNImpl
    |<----------------------:LGImpl
    |<----------------------:VKPipeline
    |<----------------------:VKDescriptorSet
    |<----------------------:VKCmdBuffer
    |<----------------------:VKRenderer
    |<----------------------:VKCmdList
    |SYDebugRendering
</pre>

## SandBox/
<pre>
    |<----------------------:Common
    |<----------------------:LGImpl
    |<----------------------:stb_image
    |SBTexturePool

    |<----------------------:Common
    |<----------------------:SNImpl
    |<----------------------:SYMeshLoading
    |<----------------------:SYMeshBatching
    |<----------------------:SYStdMeshInstanceBatching
    |<----------------------:SYWireMeshInstanceBatching
    |<----------------------:SYLightInstanceBatching
    |<----------------------:SYCameraController
    |<----------------------:SYShadowRendering
    |<----------------------:SYShadowCubeRendering
    |<----------------------:SYGDefaultRendering
    |<----------------------:SYLightRendering
    |<----------------------:SYWireRendering
    |<----------------------:SYSkyBoxRendering
    |<----------------------:SYFDefaultRendering
    |<----------------------:SYDebugRendering
    |<----------------------:SBImpl
    |<----------------------:SNType
    |<----------------------:SBComponentType
    |SBScene

    |<----------------------:Common
    |<----------------------:SNImpl
    |<----------------------:CNImpl
    |<----------------------:LGImpl
    |<----------------------:VKInstance
    |<----------------------:VKWindow
    |<----------------------:VKSurface
    |<----------------------:VKPhyDevice
    |<----------------------:VKLogDevice
    |<----------------------:VKSwapChain
    |<----------------------:VKBuffer
    |<----------------------:VKImage
    |<----------------------:VKSampler
    |<----------------------:VKRenderPass
    |<----------------------:VKFrameBuffer
    |<----------------------:VKPipeline
    |<----------------------:VKDescriptorPool
    |<----------------------:VKDescriptorSet
    |<----------------------:VKFence
    |<----------------------:VKSemaphore
    |<----------------------:VKCmdPool
    |<----------------------:VKCmdBuffer
    |<----------------------:VKRenderer
    |<----------------------:SBTexturePool
    |<----------------------:SYMeshBatching
    |<----------------------:SYStdMeshInstanceBatching
    |<----------------------:SYWireMeshInstanceBatching
    |<----------------------:SYLightInstanceBatching
    |<----------------------:SBImpl
    |<----------------------:VKCmdList
    |<----------------------:VKHelper
    |<----------------------:SBComponentType
    |<----------------------:SBRendererType
    |SBRenderer

    |<----------------------:Common
    |<----------------------:SNImpl
    |<----------------------:CNImpl
    |<----------------------:VKWindow
    |<----------------------:VKLogDevice
    |<----------------------:VKRenderer
    |<----------------------:SBTexturePool
    |<----------------------:SYStdMeshInstanceBatching
    |<----------------------:SYWireMeshInstanceBatching
    |<----------------------:SYLightInstanceBatching
    |<----------------------:SYCameraController
    |<----------------------:SYShadowRendering
    |<----------------------:SYShadowCubeRendering
    |<----------------------:SYGDefaultRendering
    |<----------------------:SYLightRendering
    |<----------------------:SYWireRendering
    |<----------------------:SYSkyBoxRendering
    |<----------------------:SYFDefaultRendering
    |<----------------------:SYDebugRendering
    |<----------------------:SNType
    |<----------------------:SBComponentType
    |<----------------------:SBRendererType
    |SBImpl

    |<----------------------:Common
    |<----------------------:SBRendererType
    |SBComponentType

    |<----------------------:Common
    |SBRendererType
</pre>

## ./
<pre>
    |<----------------------:SBScene
    |<----------------------:SBRenderer
    |<----------------------:SBImpl
    |main
</pre>