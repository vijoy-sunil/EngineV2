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
    \VKDescriptorPool

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKLogDevice
    |<----------------------:VKRenderPass
    |VKFrameBuffer          {glfw3}, {stdexcept}, {vector}, {vk_enum_string_helper}

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKLogDevice
    |<----------------------:VKRenderPass
    |VKPipeline             {glfw3}, {stdexcept}, {fstream}, {vector}, {vk_enum_string_helper}

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKLogDevice
    |<----------------------:VKDescriptorPool
    |VKDescriptorSet        {glfw3}, {stdexcept}, {vector}, {vk_enum_string_helper}

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKLogDevice
    |<----------------------:VKCmdPool
    |VKCmdBuffer            {glfw3}, {stdexcept}, {vector}, {vk_enum_string_helper}

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
    |SYMeshLoading          {stdexcept}, {string}, {unordered_map}, {utility}

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
    |<----------------------:VKWindow
    |<----------------------:VKSwapChain
    |<----------------------:SYEnum
    |<----------------------:SBComponentType
    |<----------------------:SBRendererType
    |SYCameraController     {glfw3}, {glm}, {stdexcept}, {algorithm}

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
    |SYSkyBoxRendering      {glfw3}, {stdexcept}, {string}, {vector}

    |<----------------------:SBRendererType
    |SBComponentType        {glm}, {matrix_transform}, {string}, {vector}, {algorithm}, {iterator}

    |SBRendererType         {glm}, {hash}

    |<----------------------:SNImpl
    |<----------------------:SYMeshLoading
    |<----------------------:SYMeshBatching
    |<----------------------:SYMeshInstanceBatching
    |<----------------------:SYLightInstanceBatching
    |<----------------------:SYCameraController
    |<----------------------:SYDefaultRendering
    |<----------------------:SYSkyBoxRendering
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
    |<----------------------:SYMeshBatching
    |<----------------------:SYMeshInstanceBatching
    |<----------------------:SYLightInstanceBatching
    |<----------------------:SBImpl
    |<----------------------:VKCmdList
    |<----------------------:VKHelper
    |<----------------------:SBComponentType
    |<----------------------:SBRendererType
    |SBRenderer             {glfw3}, {glm}, {string}, {unordered_map}, {vector}, {algorithm}, {cmath}

    |<----------------------:SNImpl
    |<----------------------:CNImpl
    |<----------------------:VKWindow
    |<----------------------:VKLogDevice
    |<----------------------:VKRenderer
    |<----------------------:SBTexturePool
    |<----------------------:SYMeshInstanceBatching
    |<----------------------:SYLightInstanceBatching
    |<----------------------:SYCameraController
    |<----------------------:SYDefaultRendering
    |<----------------------:SYSkyBoxRendering
    |<----------------------:SNType
    |<----------------------:SBComponentType
    |<----------------------:SBRendererType
    |SBImpl                 {glfw3}, {chrono}
</pre>

## ./
<pre>
    |<----------------------:SBScene
    |<----------------------:SBRenderer
    |<----------------------:SBImpl
    |main
</pre>