# Class/file hierarchy

## Backend/Scene/
<pre>
    |<----------------------:LGImpl
    |<----------------------:SNType
    |SNEnityMgr             {iomanip}, {array}, {queue}

    |<----------------------:SNType
    |SNComponentArrayBase

    |<----------------------|SNComponentArrayBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:SNType
    |SNComponentArray       {string}, {unordered_map}, {array}

    |<----------------------:SNComponentArrayBase
    |<----------------------:SNComponentArray
    |<----------------------:LGImpl
    |<----------------------:SNType
    |SNComponentMgr         {unordered_map}, {iomanip}

    |<----------------------:SNType
    |SNSystemBase           {set}

    |<----------------------:SNSystemBase
    |<----------------------:LGImpl
    |<----------------------:SNType
    |SNSystemMgr            {string}, {unordered_map}, {iomanip}

    |<----------------------:SNEnityMgr
    |<----------------------:SNComponentMgr
    |<----------------------:SNSystemMgr
    |<----------------------:LGImpl
    |<----------------------:LGEnum
    |<----------------------:SNType
    |SNImpl

    |SNType                 {bitset}
</pre>

## Backend/Collection/
<pre>
    |<----------------------:CNTypeInstanceBase
    |<----------------------:LGImpl
    |CNTypeInstanceArray    {string}, {unordered_map}, {iomanip}, {array}

    |<----------------------:CNTypeInstanceBase
    |<----------------------:CNTypeInstanceArray
    |<----------------------:LGImpl
    |<----------------------:LGEnum
    |CNImpl                 {unordered_map}, {iomanip}
</pre>

## Backend/Log/
<pre>
    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGEnum
    |LGImpl                 {string}, {unordered_map}, {fstream}, {sstream}, {iostream}, {iomanip}, {chrono}
</pre>

## Renderer/
<pre>
    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:LGEnum
    |VKInstance             {glfw3}, {string}, {set}, {vector}, {vk_enum_string_helper}

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |VKWindow               {glfw3}, {unordered_map}, {functional}

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKInstance
    |<----------------------:VKWindow
    |VKSurface              {glfw3}, {vk_enum_string_helper}

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKInstance
    |<----------------------:VKSurface
    |VKPhyDevice            {glfw3}, {string}, {unordered_map}, {set}, {vector}, {optional}

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKInstance
    |<----------------------:VKPhyDevice
    |VKLogDevice            {glfw3}, {set}, {vector}, {vk_enum_string_helper}

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKWindow
    |<----------------------:VKSurface
    |<----------------------:VKPhyDevice
    |<----------------------:VKLogDevice
    |<----------------------:VKHelper
    |VKSwapChain            {glfw3}, {vector}, {limits}, {algorithm}, {vk_enum_string_helper}

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKPhyDevice
    |<----------------------:VKLogDevice
    |<----------------------:VKHelper
    |                       {glfw3}, {vector}, {vk_enum_string_helper}
    \VKBuffer
    \VKImage

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKLogDevice
    |                       {glfw3}, {vk_enum_string_helper}
    \VKSampler
    \VKFence
    \VKSemaphore
    \VKCmdPool

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKLogDevice
    |                       {glfw3}, {vector}, {vk_enum_string_helper}
    \VKRenderPass
    \VKFrameBuffer
    \VKDescriptorPool
    \VKDescriptorSet
    \VKCmdBuffer

    |<----------------------|CNTypeInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKLogDevice
    |VKPipeline             {glfw3}, {fstream}, {vector}, {vk_enum_string_helper}

    |VKCmdList              {glfw3}, {vector}

    |VKHelper               {glfw3}, {set}, {vector}
</pre>