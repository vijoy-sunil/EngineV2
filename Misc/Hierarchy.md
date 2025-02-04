# Class/file hierarchy

## Backend/Layer/
<pre>
    |<----------------------:LYInstanceBase
    |<----------------------:LGImpl
    |<----------------------:LYEnum
    |LYPool                 {string}, {map}, {vector}, {functional}, {algorithm}

    |<----------------------|LYPool [PUB]
    |<----------------------:LGImpl
    |<----------------------:LYEnum
    |<----------------------:LGEnum
    |LYPoolMgr              {unordered_map}
</pre>

## Backend/Log/
<pre>
    |<----------------------|LYInstanceBase [PUB]
    |<----------------------:LGEnum
    |LGImpl                 {string}, {unordered_map}, {fstream}, {sstream}, {iostream}, {iomanip}, {chrono}
</pre>

## Renderer/
<pre>
    |<----------------------|LYInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:LGEnum
    |VKInstance             {glfw3}, {string}, {set}, {vector}, {vk_enum_string_helper}

    |<----------------------|LYInstanceBase [PUB]
    |<----------------------:LGImpl
    |VKWindow               {glfw3}, {unordered_map}, {functional}

    |<----------------------|LYInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKInstance
    |<----------------------:VKWindow
    |VKSurface              {glfw3}, {vk_enum_string_helper}

    |<----------------------|LYInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKInstance
    |<----------------------:VKSurface
    |VKPhyDevice            {glfw3}, {string}, {unordered_map}, {set}, {vector}, {optional}

    |<----------------------|LYInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKInstance
    |<----------------------:VKPhyDevice
    |VKLogDevice            {glfw3}, {set}, {vector}, {vk_enum_string_helper}

    |<----------------------|LYInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKWindow
    |<----------------------:VKSurface
    |<----------------------:VKPhyDevice
    |<----------------------:VKLogDevice
    |<----------------------:VKHelper
    |VKSwapChain            {glfw3}, {vector}, {limits}, {algorithm}, {vk_enum_string_helper}

    |<----------------------|LYInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKPhyDevice
    |<----------------------:VKLogDevice
    |<----------------------:VKHelper
    |                       {glfw3}, {vector}, {vk_enum_string_helper}
    \VKBuffer
    \VKImage

    |<----------------------|LYInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKLogDevice
    |                       {glfw3}, {vk_enum_string_helper}
    \VKSampler
    \VKFence
    \VKSemaphore
    \VKCmdPool

    |<----------------------|LYInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKLogDevice
    |                       {glfw3}, {vector}, {vk_enum_string_helper}
    \VKRenderPass
    \VKFrameBuffer
    \VKDescriptorPool
    \VKDescriptorSet
    \VKCmdBuffer

    |<----------------------|LYInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKLogDevice
    |VKPipeline             {glfw3}, {fstream}, {vector}, {vk_enum_string_helper}

    |VKCmdList              {glfw3}, {vector}

    |VKHelper               {glfw3}, {set}, {vector}
</pre>