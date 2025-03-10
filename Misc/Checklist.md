# FILES/FOLDERS

- [ ] use `#pragma once` as include guard
- [ ] namespace comment at end
- [ ] Class name match file name
- [ ] File name has to be singular, not a plural
- [ ] Parent folder/Sub folder names have to be singular
- [ ] Make sure every header file is necessary (search `std::`, respective enum headers)
- [ ] include <> paths before “” paths
- [ ] include order match inheritance order
- [ ] Global ordering
```
{
    glfw3
    glm
    stdexcept
    string
    map
    unordered_map
    fstream
    sstream
    iostream
    iomanip
    chrono
    set
    vector
    array
    queue
    functional
    optional
    limits
    algorithm
    iterator
    bitset
    vk_enum_string_helper

    Scene
    Collection
    Log
    VKInstance
    VKWindow
    VKSurface
    VKPhyDevice
    VKLogDevice
    VKSwapChain
    VKBuffer
    VKImage
    VKSampler
    VKRenderPass
    VKFrameBuffer
    VKPipeline
    VKDescriptorPool
    VKDescriptorSet
    VKFence
    VKSemaphore
    VKCmdPool
    VKCmdBuffer
    VKRenderer
    Dependency
    SandBox

    enum
    non-class files
}
```
- [ ] Make sure we are not nesting `using namespace`
- [ ] Trim trailing whitespaces
- [ ] Check all comments
- [ ] Update hierarchy file

# VARIABLES

- [ ] Use auto for vectors, structs, classes, enums (search =)
- [ ] Class object name ends with 'Obj'
- [ ] global member objects indicated by `g_`
- [ ] enums indicated by `e_`
- [ ] private member vars indicated by `m_`
- [ ] function bindings start with `run` or `destroy`, generic ones will be called `binding`
- [ ] Check if nullptr before running a binding
- [ ] vector names end with `s`
- [ ] count variables naming "name" + "sCount" (eg: instance'sCount')
- [ ] Index naming should be `idx`
- [ ] Check if uint32_t, int32_t etc. can be replaced with `using ?Type`
- [ ] `using` alias begins with upper case
- [ ] Paths as arguments should be named `?FilePath` or `?DirPath` (imageFilePath, modelFilePath etc.)
- [ ] When copying struct/vec for use as short hand, always use `auto&`
- [ ] All classes have private info struct to organize private vars
- [ ] struct names begin with upper case
- [ ] struct members should be named short (configs instead of logConfigs in LogInfo); self explanatory
- [ ] Struct info ordering
```
{
    meta
    state
    path
    resource
}
```
- [ ] Resource struct members will be
```
{
    dependency objs
    bindings
    objs created with vkCreate/vkAllocate
}
```
- [ ] bool vars shouldn't start with `is`
- [ ] use `nullptr` for null pointers, instead of `VK_NULL_HANDLE`
- [ ] For loops use matching types for iterator
- [ ] use unordered map instead of map if you don't need inputs to be inherently sorted
- [ ] make sure we are closing all open files
- [ ] make sure we are deleteing all allocated memory (search new and delete)
- [ ] image vs texture naming
```
{
    image,              when using as a collection type, or, when loading
    texture,            when prefixed with diffuse/speculat/emission, OR suffixed with idx, pool etc.
    texture image,      do not use
}
```
- [ ] mesh vs model naming
```
{
    mesh,               when related to components
    model,              when referring to loading, instances etc.
}
```

# SHADERS

- [ ] input vars begin with `i_`
- [ ] output vars begin with `o_`
- [ ] constants are capitalized

# METHODS

- [ ] Is this function necessary?
- [ ] All constant parameters passed as `const`
- [ ] Parameter names should be named to not cause confusion (for eg: device could be phy or log)
- [ ] virtual fns will start with `on` and have `override` keyword
- [ ] Struct/Vector as argument
```
{
    by value,               if copying to another struct/vector
    by pointer   (struct),  if storing its address
    by reference (vector),  if storing its address
}
```
- [ ] Constructor does
```
{
    default initializes info struct
    handle log object
    dependency objs check for nullptr
    initializes all dep objs only
}
```
- [ ] Init info method initializes all the remaining vars
- [ ] Init info method right afer constructor
- [ ] Destructor at the end of the file to destroy `new` objs created in constructor
- [ ] Common methods
```
{
    create
    get
    run
    destroy
    set
    is
    add
    remove
    handle
    init
    generate
    update
    import
    write
    save
    restore
    toggle
    populate
    on
    register
}
```
- [ ] Create vs get vs add naming
```
{
    create, if creating a struct/object
    get,    if returning an already created (elsewhere) struct/object, OR, simple fns like string gen
    add,    if creating + saving to a list
}
```
- [ ] Functions should be named long; even if it is self explanatory
- [ ] Alias functions should be named with `EXT` suffix
- [ ] Return objects/literals methods start with `get` or `create`
- [ ] Return boolean methods start with `is`
- [ ] Return struct/class/vector
```
{
    by value,                   if local
    by pointer (struct/class),  otherwise
    by reference (vector),      otherwise
}
```
- [ ] private/public/protected methods are placed appropriately
- [ ] `create` and `destroy` methods will not take any params (`init` will do it instead)
- [ ] Any methods that can be moved to helper file (used by multiple files, has no special reason to stay in module)
- [ ] Cmds will have cmd buffer as the first parameter (search `vkCmd`)
- [ ] Make sure all fns (especially ones with `delete`) are actually called

# LOG

- [ ] Check log formatting
```
    Create [O]
    Destroy [X]
    Error [?]
```
- [ ] vkcreate/vkalloc multiple objects at once will have plural msg `(s)`
- [ ] log_error followed by runtime error throwing same message
- [ ] Log file name match file name

# MISC

- [ ] `[a-z>]&`                                 use of references
- [ ] `[a-z]\*`                                 use of pointers
- [ ] `\) [a-z]`                                no use of C-style casting
- [ ] `\) &`                                    no use of C-style casting
- [ ] `[0-9]\.[0-9], [0-9]\.[0-9][0-9]`         float values end with f -- search + [space, comma, ], ), ;]
- [ ] `static`                                  is it necessary?
- [ ] `const char*`                             use const char* for string literals
- [ ] `std::string`                             use only if you require std::string features like concat, size etc.
- [ ] `.size()`                                 make sure it is casted from size_t to required types
- [ ] `size_t`                                  only used for size of an object and in fns that accept size_t
- [ ] `#endif  //`                              all endifs should have comment
- [ ] `[ ]`                                     to do list
