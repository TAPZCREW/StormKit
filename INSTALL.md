# Installation

A C++23 compliant compiler with std module support and XMake is needed to build StormKit

## Dependencies
### StormKit-core module
- [glm](https://github.com/g-truc/glm)
- [frozen](https://github.com/serge-sans-paille/frozen)
- [unordered_dense](github.com/martinus/unordered_dense)

### StormKit-image module
- [gli](https://github.com/g-truc/gli)
- [LibPNG >= 1.6](http://www.libpng.org/pub/png/libpng.html)
- [LibJPEG-turbo](https://www.libjpeg-turbo.org)

### stormkit-gpu module
- [Vulkan](https://vulkan.lunarg.com)
- [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)

## Building
```
> xmake f -m <release|debug|releasedbg> -k <shared|static> --runtimes=<c++_shared|c++_static|libstdc++_shared|libstdc++_static|MT|MD|MTd|MDd>
> xmake b
```

## Parameters
You can customize your build (with --option=value) with the following parameters 

|       Variable        |         Description             |                                   Default value                                       |
|-----------------------|---------------------------------|---------------------------------------------------------------------------------------|
| enable_tests          | Build tests                     | no                                                                                    |
| enable_examples       | Build examples                  | no                                                                                    |
| enable_log            | Build stormkit log library      | yes                                                                                   |
| enable_entities       | Build stormkit entities library | yes                                                                                   |
| enable_image          | Build stormkit image library    | yes                                                                                   |
| enable_wsi            | Build stormkit wsi library      | yes                                                                                   |
| enable_gpu            | Build stormkit gpu    library   | yes                                                                                   |
| enable_engine         | Build stormkit engine library   | yes                                                                                   |
