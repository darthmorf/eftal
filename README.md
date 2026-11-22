# eftal

A C++20 DirectX 12 renderer / engine / game, targeting Shader Model 6.0. A learning project.

**Status:** build skeleton only. Nothing opens a window or renders yet. See [TODOS.md](TODOS.md).

## Requirements

- Windows, x64
- Visual Studio 2022 with the "Desktop development with C++" workload
- [premake5](https://premake.github.io/download) (5.0 beta), placed at `Tools/premake/premake5.exe`

## Getting started

```
git clone --recursive <repo-url>
```

Already cloned without `--recursive`? Run `git submodule update --init`.

Then either:

- **`Launch.bat`** (recommended): generates the projects, opens `eftal.sln` in VS 2022 and watches for changes. Editing any `premake5.lua`, or adding/removing/renaming files under a project's `src/`, regenerates the projects automatically and VS offers to reload. The watcher runs in a minimised window and exits when VS closes.
- **`GenerateProjects.bat`**: generates the projects once, without opening VS.

## Layout

| Path | Contents |
|---|---|
| `Game/` | Application entry point (`Game.exe`). Links Core, Graphics, GLFW. |
| `Core/` | Core engine features (static lib). |
| `Graphics/` | Rendering (static lib, links Core). |
| `ThirdParty/` | Third-party code as git submodules, each with a premake script next to it (e.g. `glfw.lua`). |
| `Tools/` | premake executable and the launch/watch script. |
| `bin/<Config>/` | Final runnable output; the debugger's working directory. Generated. |

Each project keeps its sources in `<Project>/src/` and its premake script in `<Project>/premake5.lua`. Generated `.sln`/`.vcxproj` files are not committed.

## Build configurations

| | Debug | Release |
|---|---|---|
| Optimisation | Off | Speed |
| Debug info | pdb, Edit and Continue (`/ZI`) | pdb |
| Defines | `DEBUG=1`, `RELEASE=0`, `_DEBUG` | `DEBUG=0`, `RELEASE=1`, `NDEBUG` |

Use `#if DEBUG` / `#if RELEASE`. Both are always defined, and warning C4668 (undefined macro in `#if`) is an error in our code, so a typo like `#if RELAESE` fails to compile instead of silently evaluating to 0. Headers included with `<...>` are exempt.

Projects build into `<Project>/Build/Bin/<Config>/` (intermediates in `<Project>/Build/Obj/<Config>/<Project>/`). After building, `Game.exe` and `Game.pdb` are copied to `bin/<Config>/`.

Game is a windowed app (no console window) with a standard `int main()` entry point.

## Hot reload

Core and Graphics are static libraries rather than DLLs, so there's no runtime cost and the compiler can inline across them. Hot reload uses Visual Studio's built-in C++ Hot Reload (Edit and Continue) in Debug builds: while debugging, edit code and press Alt+F10.

## Third-party libraries

| Library | Version | Used for |
|---|---|---|
| [GLFW](https://www.glfw.org/) | 3.4 | Window creation and input (used with `GLFW_NO_API`; DX12 gets the HWND via `glfwGetWin32Window`) |
