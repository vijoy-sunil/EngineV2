# Class/file hierarchy
<i>Classes within {} are virtual inheritance</i>

## Layer/
<pre>
    |<----------------------:LYCommon
    |<----------------------:LYEnum
    |LYPool [PUB]           {map}, {iostream}, {functional}
    |LYPoolMgr
</pre>

## Log/
<pre>
    |<----------------------|LYCommon [PUB]
    |<----------------------:LGEnum
    |LGImpl                 {fstream}, {sstream}, {iostream}

    :LGMacro
</pre>