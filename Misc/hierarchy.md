# Class/file hierarchy

## Backend/Layer/
<pre>
    |<----------------------:LYInstanceBase
    |<----------------------:LYEnum
    |LYPool [PUB]           {map}, {vector}, {iostream}, {functional}
    |LYPoolMgr              {unordered_map}, {iostream}
</pre>

## Backend/Log/
<pre>
    |<----------------------|LYInstanceBase [PUB]
    |<----------------------:LGEnum
    |LGImpl                 {unordered_map}, {fstream}, {sstream}, {iostream}
</pre>

## Core/
<pre>
    |<----------------------|LYInstanceBase [PUB]
    |<----------------------:LGImpl
    |VKInstance             {glfw3}, {set}, {vector}, {vk_enum_string_helper}

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
    |VKPhyDevice            {glfw3}, {unordered_map}, {set}, {vector}, {optional}

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
    |<----------------------:VKHelper   {set}, {vector}
    |VKSwapChain            {glfw3}, {vector}, {vk_enum_string_helper}

    |<----------------------|LYInstanceBase [PUB]
    |<----------------------:LGImpl
    |<----------------------:VKPhyDevice
    |<----------------------:VKLogDevice
    |<----------------------:VKHelper
    |VKImage                {glfw3}, {vector}, {vk_enum_string_helper}
</pre>