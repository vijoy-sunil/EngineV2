# Class/file hierarchy

## Backend/Scene/
<pre>
    |<----------------------:LGImpl
    |<----------------------:SNType
    |SNEnityMgr             {stdexcept}, {iomanip}, {array}, {queue}

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
    |SNComponentMgr         {stdexcept}, {unordered_map}, {iomanip}

    |<----------------------:SNType
    |SNSystemBase           {set}

    |<----------------------:SNSystemBase
    |<----------------------:LGImpl
    |<----------------------:SNType
    |SNSystemMgr            {stdexcept}, {string}, {unordered_map}, {iomanip}

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
    |CNTypeInstanceArray    {stdexcept}, {string}, {unordered_map}, {iomanip}, {array}

    |<----------------------:CNTypeInstanceBase
    |<----------------------:CNTypeInstanceArray
    |<----------------------:LGImpl
    |<----------------------:LGEnum
    |CNImpl                 {stdexcept}, {string}, {unordered_map}, {iomanip}
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

## Utility/
<pre>
    |<----------------------:tiny_obj_loader
    |<----------------------:SBRendererType
    |UTModelLoader          {stdexcept}, {string}, {unordered_map}, {vector}

    |<----------------------:stb_image
    |UTImageLoader          {stdexcept}
</pre>