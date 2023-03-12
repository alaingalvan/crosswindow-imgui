# CrossWindow-ImGui

[![cmake-img]][cmake-url]
[![License][license-img]][license-url]

An *optional* library to simplify integrating [ImGui](https://github.com/ocornut/imgui) with a renderer that uses CrossWindow by providing a forked ImGui backend with **simplified defaults** and **bindings for CrossWindow events**.

> **Note** - Don't install ImGui if you're using this library, `CrossWindow-Imgui` wraps ImGui and *includes a copy*.

## Supports

- ❎ DirectX 12.x
- ⚪ OpenGL

With more graphics APIs supported in future updates.

## Installation

First add the repo as a submodule in your dependencies folder such as `external/`:

```bash
cd external
git submodule add https://github.com/alaingalvan/crosswindow-imgui.git
```

Then in your `CMakeLists.txt` file, include the following:

```cmake
add_subdirectory(external/crosswindow-imgui)

target_link_libraries(
    CrossWindowImGui
    CrossWindowGraphics
    CrossWindow
)

add_dependencies(
    CrossWindowImGui
    CrossWindowGraphics
    CrossWindow
    
)

target_link_libraries(
    ${PROJECT_NAME}
    CrossWindowImGui
)
```

## Usage

```cpp
#include "CrossWindow/CrossWindow.h"
#include "CrossWindow/ImGui.h"
#include "CrossWindow/Graphics.h"

void xmain(int argc, char** argv)
{
  // 🖼️ Create your xwin::Window...
  xwin::Window window;
  
  // 🖌️ Create your renderer...
  
#if defined(XGFX_DIRECTX12)
  
  // ❎ DirectX 12.x Swapchain
  IDXGISwapChain1* swapchain = xgfx::createSwapchain(window, factory, commandQueue, &swapchainDesc);

#endif

  // 📄 Then your API specific data structures for ImGui.

#if defined(XGFX_DIRECTX12)
  
  // ❎ DirectX 12.x
  xgfx::D3D12ImGuiManager manager;
  manager.init(device, numFramesInFlight, rtvFormat, cbvSrvHeap, fontCpuDescHandle, fontGpuDeschandle);

#endif
}

```

### Preprocessor Definitions

| CMake Options | Description |
|:-------------:|:-----------:|
| `XGFX_API` | The graphics API you're targeting, defaults to `VULKAN`, can be can be `VULKAN`, `OPENGL`, `DIRECTX12`, `METAL`, or `NONE`. |

Alternatively you can set the following preprocessor definitions manually:

| Definition | Description |
|:-------------:|:-----------:|
| `XGFX_VULKAN` |  Vulkan |
| `XGFX_OPENGL` |  OpenGL / OpenGL ES / WebGL |
| `XGFX_DIRECTX12` | DirectX 12.x |
| `XGFX_DIRECTX11` | DirectX 11.x |
| `XGFX_METAL` | Metal |

## License

CrossWindow-ImGui is licensed as either **MIT** or **Apache-2.0**, whichever you would prefer.

[cmake-img]: https://img.shields.io/badge/cmake-3.6-1f9948.svg?style=flat-square
[cmake-url]: https://cmake.org/
[license-img]: https://img.shields.io/:license-mit-blue.svg?style=flat-square
[license-url]: https://opensource.org/licenses/MIT
