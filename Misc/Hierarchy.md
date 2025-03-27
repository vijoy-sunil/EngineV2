# Class/file hierarchy

## Backend/Scene/
<pre>
    |<----------------------:LGImpl
    |<----------------------:SNType
    |SNEnityMgr             {stdexcept}, {array}, {queue}

    |<----------------------:SNType
    |SNComponentArrayBase

    |<----------------------|SNComponentArrayBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:SNType
    |SNComponentArray       {stdexcept}, {string}, {unordered_map}, {array}

    |<----------------------:SNComponentArrayBase
    |<----------------------:SNComponentArray
    |<----------------------:LGImpl
    |<----------------------:SNType
    |SNComponentMgr         {stdexcept}, {unordered_map}

    |<----------------------:SNType
    |SNSystemBase           {set}

    |<----------------------:SNSystemBase
    |<----------------------:LGImpl
    |<----------------------:SNType
    |SNSystemMgr            {stdexcept}, {string}, {unordered_map}

    |<----------------------:SNEnityMgr
    |<----------------------:SNComponentMgr
    |<----------------------:SNSystemMgr
    |<----------------------:LGImpl
    |<----------------------:SNType
    |SNImpl

    |SNType                 {bitset}
</pre>

## Backend/Collection/
<pre>
    |<----------------------:CNTypeInstanceBase
    |<----------------------:LGImpl
    |CNTypeInstanceArray    {stdexcept}, {string}, {unordered_map}, {array}

    |<----------------------:CNTypeInstanceBase
    |<----------------------:CNTypeInstanceArray
    |<----------------------:LGImpl
    |CNImpl                 {stdexcept}, {string}, {unordered_map}
</pre>

## Backend/Log/
<pre>
    |<----------------------|CNTypeInstanceBase [PUB]
    |LGImpl                 {string}, {unordered_map}, {fstream}, {sstream}, {iostream}, {iomanip}, {chrono}
</pre>

## Backend/Renderer/
<pre>
    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |VKInstance             {glfw3}, {stdexcept}, {string}, {set}, {vector}, {vk_enum_string_helper}

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |VKWindow               {glfw3}, {unordered_map}, {functional}

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKInstance
    |<----------------------:VKWindow
    |VKSurface              {glfw3}, {stdexcept}, {vk_enum_string_helper}

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKInstance
    |<----------------------:VKSurface
    |VKPhyDevice            {glfw3}, {stdexcept}, {string}, {unordered_map}, {set}, {vector}, {optional}

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKInstance
    |<----------------------:VKPhyDevice
    |VKLogDevice            {glfw3}, {stdexcept}, {set}, {vector}, {vk_enum_string_helper}

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKWindow
    |<----------------------:VKSurface
    |<----------------------:VKPhyDevice
    |<----------------------:VKLogDevice
    |<----------------------:VKHelper
    |VKSwapChain            {glfw3}, {stdexcept}, {vector}, {limits}, {algorithm}, {vk_enum_string_helper}

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKPhyDevice
    |<----------------------:VKLogDevice
    |<----------------------:VKHelper
    |                       {glfw3}, {stdexcept}, {vector}, {vk_enum_string_helper}
    \VKBuffer
    \VKImage

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKLogDevice
    |                       {glfw3}, {stdexcept}, {vk_enum_string_helper}
    \VKSampler
    \VKFence
    \VKSemaphore
    \VKCmdPool

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKLogDevice
    |                       {glfw3}, {stdexcept}, {vector}, {vk_enum_string_helper}
    \VKRenderPass
    \VKFrameBuffer
    \VKDescriptorPool
    \VKDescriptorSet
    \VKCmdBuffer

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKLogDevice
    |VKPipeline             {glfw3}, {stdexcept}, {fstream}, {vector}, {vk_enum_string_helper}

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKWindow
    |<----------------------:VKLogDevice
    |<----------------------:VKSwapChain
    |<----------------------:VKFence
    |<----------------------:VKSemaphore
    |<----------------------:VKCmdBuffer
    |<----------------------:VKHelper
    |VKRenderer             {glfw3}, {stdexcept}, {vector}, {functional}, {vk_enum_string_helper}

    |VKCmdList              {glfw3}, {stdexcept}, {vector}

    |VKHelper               {glfw3}, {stdexcept}, {set}, {vector}
</pre>

## SandBox/
<pre>
    |<----------------------:LGImpl
    |<----------------------:stb_image
    |<----------------------:SBRendererType
    |SBTexturePool          {stdexcept}, {string}, {map}, {unordered_map}

    |<----------------------|SNSystemBase [PUB]
    |<----------------------:SNImpl
    |<----------------------:LGImpl
    |<----------------------:tiny_obj_loader
    |<----------------------:SBTexturePool
    |<----------------------:SBComponentType
    |<----------------------:SBRendererType
    |SYMeshLoading          {stdexcept}, {string}, {unordered_map}

    |<----------------------|SNSystemBase [PUB]
    |<----------------------:SNImpl
    |<----------------------:LGImpl
    |<----------------------:SNType
    |<----------------------:SBComponentType
    |<----------------------:SBRendererType
    |SYMeshBatching         {stdexcept}, {unordered_map}, {vector}

    |<----------------------|SNSystemBase [PUB]
    |<----------------------:SNImpl
    |<----------------------:LGImpl
    |<----------------------:SNType
    |<----------------------:SBComponentType
    |SYMeshInstanceBatching {glm}, {stdexcept}, {unordered_map}, {vector}

    |<----------------------|SNSystemBase [PUB]
    |<----------------------:SNImpl
    |<----------------------:LGImpl
    |<----------------------:SNType
    |<----------------------:SBComponentType
    |<----------------------:SBRendererType
    |SYLightInstanceBatching {glm}, {stdexcept}, {unordered_map}, {vector}

    |<----------------------|SNSystemBase [PUB]
    |<----------------------:SNImpl
    |<----------------------:CNImpl
    |<----------------------:LGImpl
    |<----------------------:VKSwapChain
    |<----------------------:SBComponentType
    |<----------------------:SBRendererType
    |SYCameraController     {glm}, {stdexcept}

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
    |SYDefaultRendering     {glfw3}, {stdexcept}, {string}, {vector}

    |SBRendererType         {glm}, {hash}

    |<----------------------:SNImpl
    |<----------------------:SYMeshLoading
    |<----------------------:SYMeshBatching
    |<----------------------:SYMeshController
    |<----------------------:SYMeshInstanceBatching
    |<----------------------:SYLightController
    |<----------------------:SYLightInstanceBatching
    |<----------------------:SYCameraController
    |<----------------------:SYDefaultRendering
    |<----------------------:SBImpl
    |<----------------------:SNType
    |<----------------------:SBComponentType
    |SBScene                {glm}, {string}, {vector}, {utility}

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
    |<----------------------:SYMeshLoading
    |<----------------------:SYMeshBatching
    |<----------------------:SYMeshInstanceBatching
    |<----------------------:SYLightInstanceBatching
    |<----------------------:SBImpl
    |<----------------------:VKCmdList
    |<----------------------:VKHelper
    |<----------------------:SBRendererType
    |SBRenderer             {glfw3}, {string}, {unordered_map}, {vector}, {algorithm}, {cmath}
</pre>