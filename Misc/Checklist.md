# FILES/FOLDERS

- [x] use `#pragma once` as include guard
- [x] namespace comment at end
- [x] Class name match file name
- [x] File name has to be singular, not a plural
- [x] Parent folder/Sub folder names have to be singular
- [x] Make sure every header file is necessary (search `std::`, respective enum headers)
- [x] include <> paths before “” paths
- [x] include order match inheritance order
- [x] Global ordering
```
{
    glfw3
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
- [x] Class object name ends with 'Obj'
- [x] global member objects indicated by `g_`
- [x] enums indicated by `e_`
- [x] private member vars indicated by `m_`
- [x] function bindings start with `run` or `destroy`, generic ones will be called `binding`
- [x] Check if nullptr before running a binding
- [x] vector names end with `s`
- [x] count variables naming "name" + "sCount" (eg: instance'sCount')
- [x] Index naming should be `idx`
- [x] `using` alias begins with upper case
- [x] When copying struct/vec for use as short hand, always use `auto&`
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
- [x] use `nullptr` for null pointers, instead of `VK_NULL_HANDLE`
- [x] For loops use matching types for iterator
- [x] use unordered map instead of map if you don't need inputs to be inherently sorted
- [x] make sure we are closing all open files
- [x] make sure we are deleteing all allocated memory (search new and delete)

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
- [x] `create` and `destroy` functions at the end of file before virtual overrides and destructor
- [x] `create` and `destroy` methods will not take any params (`init` will do it instead)
- [x] Any methods that can be moved to helper file (used by multiple files, has no special reason to stay in module)
- [x] Cmds will have cmd buffer as the first parameter (search `vkCmd`)
- [ ] Make sure all fns (especially ones with `delete`) are actually called

# LOG

- [x] Check log formatting
```
    Create [O]
    Destroy [X]
    Error [?]
```
- [x] vkcreate/vkalloc multiple objects at once will have plural msg `(s)`
- [x] log_error followed by runtime error throwing same message
- [x] Log file name match file name

# MISC

- [ ] `[a-z]<`                                  search all “no space before <”
- [ ] `->[^ ]`                                  search all “no space after ->”
- [ ] `[^ ] \(\)`                               search all “space before ()”
- [ ] `[^ ]\([a-z]`                             search all “”no space before (a-z)”
- [ ] `[^ ] \[`                                 search all “space before [”
- [ ] `[a-z>]&`                                 use of references
- [ ] `[a-z]\*`                                 use of pointers
- [ ] `\) [a-z]`                                no use of C-style casting
- [ ] `\) &`                                    no use of C-style casting
- [ ] `[0-9]\.[0-9], [0-9]\.[0-9][0-9]`         float values end with f -- search + [space, comma, ], ), ;]
- [ ] `() {`                                    void parameter
- [ ] `auto&`                                   use of auto&
- [ ] `static`                                  is it necessary?
- [ ] `const char*`                             use const char* for string literals
- [ ] `std::string`                             use only if you require std::string features like concat, size etc.
- [ ] `uint32_t`                                we usually use int32_t
- [ ] `.size()`                                 make sure it is casted from size_t to required types
- [ ] `c_str()`                                 when passing it to const char* parameter
- [ ] `size_t`                                  only used for size of an object and in fns that accept size_t
- [ ] `#endif  //`                              all endifs should have comment
- [ ] `" [", "[ ", " [ ", " ]", "] ", " ] "`    spacing around log brackets
- [ ] `[ ]`                                     to do list
