# FILES/FOLDERS

- [ ] use `#pragma` once as include guard
- [ ] namespace comment at end
- [ ] Class name match file name
- [ ] Base class name ends with `Base`
- [ ] File name has to be singular, not a plural
- [ ] Parent folder/Sub folder names have to be singular
- [ ] Make sure every header file is necessary
- [ ] include <> paths before “” paths
- [ ] include order match inheritance order
- [ ] Global ordering
```
{
    Layer
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
}
```
- [ ] Make sure we are not nesting `using namespace`
- [ ] Trim trailing whitespaces
- [ ] Check all comments

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
- [ ] Resource struct members will be dependency objs, bindings, objs created with vkCreate/vkAllocate
- [ ] bool vars shouldn't start with `is`
- [ ] use `nullptr` for null pointers, instead of `VK_NULL_HANDLE`
- [ ] For loops use matching types for iterator
- [ ] use unordered map instead of map if you don't need inputs to be inherently sorted
- [ ] make sure we are closing all open files
- [ ] make sure we are deleteing all allocated memory (search new and delete)

# METHODS

- [ ] Is this function necessary?
- [ ] All constant parameters passed as `const`
- [ ] Struct/Vector as argument
```
{
    by value,               if copying to another struct/vector
    by pointer   (struct),  if storing its address
    by reference (vector),  if storing its address
}
```
- [ ] Constructor default initializes
```
{
    info struct
    handle log object
    initializes all dep objs only
}
```
- [ ] Init info method initializes all the remaining vars
- [ ] Init info method right afer constructor
- [ ] Dependency objs check for `nullptr` in constructor
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
}
```
- [ ] Create vs get vs add naming
```
{
    create, if creating a struct/object
    get,    if returning an already created (elsewhere) struct/object
    add,    if creating + saving to a list
}
```
- [ ] Functions should be named long; even if it is self explanatory
- [ ] Parameter names should be named to not cause confusion (for eg: device could be phy or log)
- [ ] Alias functions should be named with `EXT` suffix
- [ ] Return objects/literals methods start with `get`
- [ ] Return boolean methods start with `is`
- [ ] Return structure/class by pointer
- [ ] Return vector
```
{
    by value,       if local
    by reference,   otherwise
}
```
- [ ] private/public/protected methods are placed appropriately
- [ ] `create` and `destroy` functions at the end of file before destructor (since they are layer instance bindings)
- [ ] `create` and `destroy` methods will not take any params (`init` will do it instead)
- [ ] Any methods that can be moved to helper file (used by multiple files, has no special reason to stay in module)

# LOG

- [ ] Check log formatting
```
    Create [O]
    Destroy [X]
    Error [?]
```
- [ ] log_error followed by runtime error throwing same message
- [ ] Log file name match file name

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
- [ ] `[ X ]`                                   to do list
