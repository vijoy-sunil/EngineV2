# FILES/FOLDERS

- [x] use `#pragma once` as include guard
- [x] namespace comment at end
- [x] Class name match file name
- [x] File name has to be singular, not a plural
- [x] Parent folder/Sub folder names have to be singular
- [x] Make sure every header file is necessary (search `::`)
- [x] include <> paths before “” paths
- [x] include order match inheritance order
- [x] Global ordering
```
{
    Common
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
- [x] Make sure we are not nesting `using namespace`
- [x] Trim trailing whitespaces
- [x] Check all comments
- [x] Update hierarchy file

# VARIABLES

- [x] Use auto for vectors, structs, classes, enums (search =)
- [x] Class object name ends with `Obj`
- [x] global member objects indicated by `g_`
- [x] enums indicated by `e_`
- [x] private member vars indicated by `m_`
- [x] function bindings will be called `?Binding`
- [x] Check if `nullptr` before running a binding
- [x] vector names end with `s`
- [x] count variables name should be suffixed with `sCount`
- [x] Index naming should be `idx`
- [x] `Offset` shouldn't be suffixed with `idx`
- [x] Check if `uint32_t`, `int32_t` etc. can be replaced with `using ?Type` (search `int`)
- [x] `using` alias begins with upper case
- [x] Paths as arguments should be named `?FilePath` or `?DirPath` (imageFilePath, modelFilePath etc.)
- [x] When copying struct/vec for use as short hand, always use `auto&`
```
{
    must match naming, or
    use of reference must be justified
}
```
- [x] All classes have private info struct to organize private vars
- [x] struct names begin with upper case
- [x] struct members should be named short (configs instead of logConfigs in LogInfo); self explanatory
- [x] Struct info ordering
```
{
    meta
    state
    path
    resource
}
```
- [x] Resource struct members will be
```
{
    dependency objs
    bindings
    objs created with vkCreate/vkAllocate
}
```
- [x] bool vars shouldn't start with `is`
- [x] bool vars should generally be in `State` struct
- [x] use `nullptr` for null pointers, instead of `VK_NULL_HANDLE`
- [x] For loops use matching types for iterator
- [x] use `unordered_map` instead of `map` if you don't need inputs to be inherently sorted
- [x] make sure we are closing all open files
- [x] make sure we are deleteing all allocated memory (search new and delete)
- [x] image vs texture naming
```
{
    image,              when using as a collection type, or, when loading
    texture,            when prefixed with diffuse/speculat/emission, OR suffixed with idx, pool etc.
    texture image,      do not use
}
```
- [x] mesh vs model naming
```
{
    mesh,               when related to components, instances
    model,              file path, matrix etc.
}
```

# SHADERS

- [x] input vars begin with `i_`
- [x] output vars begin with `o_`
- [x] constants are capitalized
- [x] array names end with `s` (search `[`)

# METHODS

- [x] Is this function necessary?
- [x] All constant parameters passed as `const`
- [x] Parameter names should be named to not cause confusion (for eg: device could be phy or log)
- [x] virtual fns will start with `on` and have `override` keyword
- [x] Struct/Vector as argument
```
{
    by value,               if copying to another struct/vector
    by pointer   (struct),  if storing its address
    by reference (vector),  if storing its address
}
```
- [x] Constructor does
```
{
    default initializes info struct
    handle log object
    dependency objs check for nullptr
    initializes all dep objs only
}
```
- [x] Init info method initializes all the remaining vars
- [x] Init info method right afer constructor
- [x] Destructor at the end of the file to destroy `new` objs created in constructor
- [x] Common methods
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
- [x] Create vs get vs add naming
```
{
    create, if creating a struct/object
    get,    if returning an already created (elsewhere) struct/object, OR, simple fns like string gen
    add,    if creating + saving to a list
}
```
- [x] Functions should be named long; even if it is self explanatory
- [x] Alias functions should be named with `EXT` suffix
- [x] Return objects/literals methods start with `get` or `create`
- [x] Return boolean methods start with `is`
- [x] Return struct/class/vector
```
{
    by value,                   if local
    by pointer (struct/class),  otherwise
    by reference (vector),      otherwise
}
```
- [x] private/public/protected methods are placed appropriately
- [x] `create` and `destroy` methods will not take any params (`init` will do it instead)
- [x] Any methods that can be moved to helper file (used by multiple files, has no special reason to stay in module)
- [x] Cmds will have cmd buffer as the first parameter (search `vkCmd`)
- [x] Make sure all fns (especially ones with `delete`) are actually called

# LOG

- [x] Check log formatting
```
    Create [O]
    Destroy [X]
    Error [?]
```
- [x] vkcreate/vkalloc multiple objects at once will have plural msg `(s)`
- [x] `LOG_ERROR` followed by runtime error throwing same message
- [x] Log file name match file name

# MISC

- [x] `\) [a-z]`                                no use of C-style casting
- [x] `\) &`                                    no use of C-style casting
- [x] `[0-9]\.[0-9], [0-9]\.[0-9][0-9]`         float values end with `f`
- [x] `static`                                  is it necessary?
- [x] `const char*`                             use `const char*` for string literals
- [x] `std::string`                             use only if you require `std::string` features like concat, size etc.
- [x] `.size()`                                 make sure it is casted from `size_t` to required types
- [x] `size_t`                                  only used for size of an object and in fns that accept `size_t`
- [x] `#endif  //`                              all `endif`s should have comment
- [x] `++`, `--`                                prefer prefix over postfix unless in `for` loop
- [x] `::`                                      avoid namespace resolution in the same namespace
- [x] `vec4`                                    host shouldn't be handling `vec4`
- [x] `[ ]`                                     to do list
