# CLAUDE.md

Persistent context for Claude. Not human-facing documentation (that's README.md). Keep it short and current.

## Working rules (highest priority)
- Never edit files unless explicitly asked to. "Is this right?" / "fix it" about a pasted snippet means answer inline with corrected code. When in doubt, answer inline.
- Approval for one edit does not cover the next. For non-trivial changes, propose first and wait.
- Answer only what was asked. Don't list follow-up issues or next steps unless prompted - the developer is often midway through an implementation. Exception: when asked to review, mention everything relevant.
- Learning project: the developer writes most code and wants to understand and own every line. Prefer small, reviewable steps; explain the *why* for non-obvious DX12 decisions (resource states, barriers, descriptors, sync) in the answer, not in code comments.
- Don't add libraries, frameworks, or abstractions without asking. For meaningful design choices, give a recommendation with brief trade-offs and let the developer decide.
- Token budget: developer is on the Claude Pro plan (usage capped per session and weekly, shared with claude.ai). Use targeted reads/greps, don't re-read files already in context, no subagents or web fetches unless needed or asked, keep answers concise. Flag expensive tasks and suggest a cheaper scope first.

## Project
- eftal: C++20 DX12 renderer / engine / game, Shader Model 6.0 (HLSL via DXC), x64 Windows only.
- Developer follows the 3dgep "Learning DirectX 12" tutorial (Lesson 1 onwards), cross-checked against Microsoft's DirectX-Graphics-Samples. Expect questions in that order.
- Status:
  - `Game/Main.cpp`: GLFW window + D3D12 clear-colour loop (device, 3-buffer flip swap chain, RTV heap, per-frame allocators, one command list, fence sync, debug layers + info queues). All rendering code is still here.
  - `Core/Logging.{h,cpp}`: static `Logging` class. `Log(LogCat, fmt, ...)` printf-style; categories (`LogCat`) map to a 4-char name + console colour. Output: `OutputDebugStringA` if a debugger is attached; spawned console (`AllocConsole`, ANSI colours) in Debug. No file output yet (see TODOS.md).
  - `Core/Macros.h`: shared utility macros (`ARRAY_COUNT`, `CLAMP`).
  - `Core/Core.cpp`, `Graphics/Graphics.cpp`: empty placeholders. Splitting Main.cpp into Core/Graphics hasn't been designed - ask before restructuring.

## Environment and build
- Repo is on Windows; Claude runs in WSL2 Ubuntu via `claude.bat`. Builds/runs happen in VS2022 on Windows; Claude can't build. Ask before assuming a toolchain.
- Git branch `main`, remote `origin` on GitHub. Submodules: ThirdParty/glfw (3.4), ThirdParty/DirectX-Headers (v1.721.2).
- Premake5 (`Tools/premake/premake5.exe`). `Launch.bat` runs premake, opens eftal.sln, and regenerates on `.lua` edits or added/removed/renamed `.h`/`.cpp` files (`Tools/Launch.ps1`). `GenerateProjects.bat` regenerates once.
- Output layout: exe + pdb in `Bin/<Config>/` (debug working dir); static libs in `Build/Lib/<Config>/`; intermediates in `Build/Obj/<Config>/<Project>/`; generated .vcxproj in `Build/Projects/`; .sln in root. All generated files gitignored.
- Workspace-wide defines: `DEBUG`/`RELEASE` (0/1), `NOMINMAX`, `WIN32_LEAN_AND_MEAN`.
- Game is a `WindowedApp` with `entrypoint "mainCRTStartup"` (keeps `main()`), so no console unless the logger spawns one.
- VS Output window: `OutputDebugString` text needs right-click > "Program Output" enabled.

## Code conventions
- Existing code overrides anything here. Code in `Core/` is the newest and best reflects the developer's style.
- Core style: tabs; PascalCase types and functions; `mMember`, `inParam`, `gGlobal` prefixes; camelCase locals; `UPPER_SNAKE` for macros and `static constexpr` constants; `static` for file-local globals/tables.
- `Game/Main.cpp` (older, tutorial-derived): 4-space indent, `gGlobal` prefix, `ComPtr` for all COM objects, `CHECK_HR(expr)` for HRESULTs (temporary, only usable in `main()`).
- No C++ standard library (`std::string`, `std::format`, containers, ...): performance and memory control. A custom replacement comes later; use fixed buffers / C-style code. CRT functions (`snprintf`, `fputs`, ...) are in use in Logging but not formally approved - ask before relying on new ones.
- Sources live directly in `<Project>/` (no `src/`). Repo root is the include dir, so includes are module-qualified: `#include <Core/Logging.h>`. Headers must compile on their own (e.g. `Logging.h` includes `<sal.h>` for `_Printf_format_string_`).
- DirectX includes: `<directx/d3d12.h>` / `<directx/d3dx12.h>` (DirectX-Headers submodule), not the SDK's `<d3d12.h>`.
- Third-party code: git submodule in `ThirdParty/` with its premake script beside it (`ThirdParty/<name>.lua`), never inside the submodule.
- Deferred work: `// TODO_<AREA>` comments in code (TODO_ASSERT, TODO_LOGGING, TODO_MATHS_LIB, ...), each area listed in TODOS.md.
- Comments: sparse. Only non-obvious intent or constraints. No restating the code, no banners, no "This function..." boilerplate.

## Decisions log
Date, decision, short reason. Chronological.
- 2026-11-22: DX12, SM 6.0 baseline to support older GPUs. Descriptor-table binding on all hardware (no bindless).
- 2026-11-22: FUTURE PLAN, not implemented: higher-SM features as optional runtime-detected tiers with raster fallbacks (first candidate: DXR inline raytracing, SM 6.5, for shadows). Don't build towards this until the baseline renderer exists.
- 2026-11-22: Premake5, VS2022, C++20, x64 only. Developer choice.
- 2026-11-22: Core/Graphics are static libs, not DLLs. Hot reload via MSVC Edit and Continue (Debug, /ZI). No runtime cost, allows cross-lib inlining/LTO.
- 2026-11-22: `DEBUG`/`RELEASE` always defined as 0/1 for `#if`; C4668 is an error (external `<...>` headers and ThirdParty projects exempt).
- 2026-11-22: Windowing via GLFW 3.4 (`GLFW_NO_API`, HWND via `glfwGetWin32Window`). Small, easy to build with premake. SDL3 rejected for now.
- 2026-11-22: DirectX-Headers submodule (header-only, no premake project). Game links d3d12, dxgi, dxguid. Agility SDK/NuGet/vcpkg not needed at SM 6.0.
- 2026-11-23: Project layout and output dirs as described under Environment. Keeps project folders source-only.
- 2026-11-23: Avoid C++ stdlib (see Code conventions).
- 2026-11-23: Logging is printf-style varargs into a fixed stack buffer. Colour (ANSI codes) only on the console, never in the buffer sent to `OutputDebugStringA`. `Log` appends the newline; call sites don't.
- 2026-11-23: `NOMINMAX` and `WIN32_LEAN_AND_MEAN` workspace-wide. Include excluded Win32 headers (winsock, shellapi, mmsystem, ...) explicitly if needed.

## Open questions
- Minimum feature level: code uses 12_0 (guarantees resource binding tier 2). Suggested, not formally confirmed.
- Error handling / asserts (TODO_ASSERT). Currently `CHECK_HR` plus `return -1`.
- Logging: file output location and flush policy, buffering, levels vs categories (see TODOS.md).
- Remaining third-party deps (D3D12MA, DirectXTex, ImGui) and the maths library (custom vs DirectXMath; see TODO_MATHS_LIB).
- How to split Main.cpp into Core/Graphics.
- CMake-only third-party libs: premake `kind "Makefile"` project running CMake, or prebuilt libs? Discussed, not decided.
- Shader compilation: offline DXC vs runtime?
- Whether Main.cpp should be converted to the Core style (tabs etc.).

## Maintaining this file
- Update when decisions are made, structure changes, or something surprises Claude. Remove stale info rather than appending contradictions.
- Explicit developer instructions about how to work or conventions: add here directly (developer reviews via git).
- Edits Claude infers on its own (recurring corrections, observed conventions): propose first.
