# Class/file hierarchy

## Backend/Layer/
<pre>
    |<----------------------:LYCommon
    |<----------------------:LYEnum
    |LYPool [PUB]           {map}, {iostream}, {functional}
    |LYPoolMgr
</pre>

## Backend/Log/
<pre>
    |<----------------------|LYCommon [PUB]
    |<----------------------:LGEnum
    |LGImpl                 {map}, {fstream}, {sstream}, {iostream}
</pre>

## Core/
<pre>
    |<----------------------|LYCommon [PUB]
    |<----------------------:LGImpl
    |VKWindow               {glfw3}, {map}, {functional}
</pre>