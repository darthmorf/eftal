# eftal

A C++20 DirectX 12 renderer / engine / game, targeting Shader Model 6.0. A learning project.

**Status:** opens a GLFW window and clears it to a colour with D3D12 every frame. The renderer currently lives in `Game/Main.cpp`; Core has a logger, Graphics is still empty. See [TODOS.md](TODOS.md).

## Requirements

- Windows 10 or 11, x64
- A D3D12 GPU supporting feature level 12_0 (roughly NVIDIA GTX 900 / AMD GCN 2 / Intel Skylake or newer)
- Visual Studio 2022 with the "Desktop development with C++" workload
- [premake5](https://premake.github.io/download) (5.0 beta), placed at `Tools/premake/premake5.exe`

## Getting started

```
git clone --recursive https://github.com/darthmorf/eftal.git
```

Already cloned without `--recursive`? Run `git submodule update --init`.

Then either:

- **`Launch.bat`** (recommended): generates the projects, opens `eftal.sln` in VS 2022 and watches for changes. Editing any `premake5.lua`, or adding/removing/renaming `.h`/`.cpp` files in a project, regenerates the projects automatically and VS offers to reload. The watcher runs in a minimised window and exits when VS closes.
- **`GenerateProjects.bat`**: generates the projects once, without opening VS.

## Layout

| Path | Contents |
|---|---|
| `Game/` | Application entry point (`Game.exe`). Links Core, Graphics, GLFW and the D3D12/DXGI system libs. |
| `Core/` | Core engine features (static lib): logging, shared macros. |
| `Graphics/` | Rendering (static lib, links Core). |
| `ThirdParty/` | Third-party code as git submodules, each with a premake script next to it (e.g. `glfw.lua`). |
| `Tools/` | premake executable and the launch/watch script. |
| `Bin/<Config>/` | `Game.exe` + pdb; the debugger's working directory. Generated. |
| `Build/` | Generated projects (`Build/Projects/`), static libs (`Build/Lib/<Config>/`) and intermediates (`Build/Obj/`). |

Each project keeps its sources and its `premake5.lua` directly in `<Project>/`. The repo root is the include directory, so includes name the project: `#include <Core/Logging.h>`. Generated files (`.sln`, `Bin/`, `Build/`) are not committed.

## Build configurations

| | Debug | Release |
|---|---|---|
| Optimisation | Off | Speed |
| Debug info | pdb, Edit and Continue (`/ZI`) | pdb |
| Defines | `DEBUG=1`, `RELEASE=0`, `_DEBUG` | `DEBUG=0`, `RELEASE=1`, `NDEBUG` |

Both configurations also define `NOMINMAX` and `WIN32_LEAN_AND_MEAN`.

Use `#if DEBUG` / `#if RELEASE`. Both are always defined, and warning C4668 (undefined macro in `#if`) is an error in our code, so a typo like `#if RELAESE` fails to compile instead of silently evaluating to 0. Headers included with `<...>` are exempt.

Game is a windowed app (still using `main()`), so it has no console of its own. See Logging below.

## Renderer (current state)

- **Device:** picks the highest-performance hardware adapter (`IDXGIFactory6::EnumAdapterByGpuPreference`), minimum feature level 12_0.
- **Swap chain:** 3 back buffers, flip-discard, created from the GLFW window's HWND. Alt+Enter handling by DXGI is disabled.
- **Per frame:** one command allocator per back buffer, a single reused command list, `PRESENT` → `RENDER_TARGET` → clear → `PRESENT` barriers, then execute and present.
- **Synchronisation:** one fence; each frame slot stores the fence value signalled after its work, and the CPU waits on it before reusing that slot. The GPU is flushed before shutdown.

### Debugging

In Debug builds the D3D12 and DXGI debug layers are enabled:

- When a debugger is attached, errors (and D3D12 warnings) break inside the offending API call. VS reports this as exception `0x0000087A`; that is the debug layer, not a crash.
- `CHECK_HR(expr)` wraps calls returning `HRESULT`. On failure it logs the failing call and HRESULT, breaks if a debugger is attached, and returns from `main`.

## Logging

`Logging::Log(LogCat::DX12, "format %d", value)` - printf-style, with a category (`Misc`, `Assert`, `GLFW`, `DX12`, `Debug`) shown as a `[NAME]` prefix. A newline is added automatically.

- With a debugger attached, output goes to Visual Studio's Output window (enable right-click > "Program Output").
- In Debug builds, a console window is spawned (by default always; otherwise only without a debugger), with a colour per category.
- Writing to a log file is planned (see TODOS.md).

## Hot reload

Core and Graphics are static libraries rather than DLLs, so there's no runtime cost and the compiler can inline across them. Hot reload uses Visual Studio's built-in C++ Hot Reload (Edit and Continue) in Debug builds: while debugging, edit code and press Alt+F10.

## Third-party libraries

| Library | Version | Used for |
|---|---|---|
| [GLFW](https://www.glfw.org/) | 3.4 | Window creation and input (used with `GLFW_NO_API`; DX12 gets the HWND via `glfwGetWin32Window`) |
| [DirectX-Headers](https://github.com/microsoft/DirectX-Headers) | 1.721.2 | Header-only: `d3dx12.h` helpers and newer `d3d12.h`. Include as `<directx/d3d12.h>` / `<directx/d3dx12.h>`, not the Windows SDK's `<d3d12.h>`. |
