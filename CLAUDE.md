# CLAUDE.md

Persistent context for Claude. Not human-facing documentation. Keep it short and current.

## Project
- Name: eftal
- C++ DX12 renderer / engine / game. Target: Shader Model 6.0 (HLSL compiled with DXC).
- Status: GLFW window + D3D12 clear-colour loop (device, 3-buffer flip swap chain, RTV heap, per-frame allocators, one command list, fence sync, debug layers). All of it is in `Game/src/Main.cpp`; Core and Graphics are empty placeholders. Splitting into Core/Graphics hasn't been designed yet - ask before restructuring.
- Developer follows the 3dgep "Learning DirectX 12" tutorial (Lesson 1 onwards), cross-checked against Microsoft's DirectX-Graphics-Samples. Expect questions in that order; the developer writes most code and asks for review/explanations.
- Purpose: a learning project. The developer wants to understand and own every line. Code quality and understanding matter more than speed of delivery.

## Environment
- Repo lives on Windows at this directory; Claude runs in WSL2 Ubuntu via `claude.bat`.
- DX12 builds/runs on Windows, not in WSL. Building or running from here would go through Windows interop (`cmd.exe`, `cl.exe`, `cmake.exe`, etc.) - not set up yet. Ask before assuming a toolchain.
- Git branch is `main`, remote `origin` on GitHub. Submodules: ThirdParty/glfw (3.4), ThirdParty/DirectX-Headers (v1.721.2).
- Developer opens VS via `Launch.bat` (runs premake, opens eftal.sln, watches .lua and src/ changes to regenerate). Generated .sln/.vcxproj are gitignored.
- VS Output window: debug-layer/`OutputDebugString` text needs right-click > "Program Output" enabled. Game is a ConsoleApp, so stdout/stderr go to its console.

## Usage budget
- Developer is on the Claude Pro plan: usage is capped per session window and weekly, shared across claude.ai and Claude Code. See https://support.claude.com/en/articles/8325606-what-is-the-pro-plan
- Be economical with tokens: targeted reads/greps over whole-file or repo-wide sweeps, don't re-read files already in context, no subagents or web fetches unless clearly needed or asked for, keep responses concise.
- If a task looks expensive (large refactor, broad research, long debugging loop), say so and suggest a cheaper approach or scope before starting.

## How to work with this developer
- Always propose changes and wait for approval before editing code, unless the developer explicitly says to go ahead for a given task. Approval for one change does not cover the next.
- Prefer small, reviewable steps over large drops of code. Explain the *why* for non-obvious DX12 decisions (resource states, barriers, descriptor heaps, synchronization) when proposing, not in code comments.
- Don't add libraries, frameworks, or abstractions without asking.
- When there are meaningful design choices (e.g. ownership model, error handling, abstraction layer over D3D12), present a recommendation with brief trade-offs and let the developer decide.

## Code style
- Match existing code in the repo once it exists; existing code overrides anything here.
- Observed conventions in Main.cpp: camelCase locals, `g_` prefix for file-scope globals, `ComPtr` for all COM objects, `CHECK_HR(expr)` for HRESULTs (temporary, main()-only), DirectX includes as `<directx/d3d12.h>` / `<directx/d3dx12.h>`.
- Deferred work is tagged in code as `// TODO_<AREA>` (e.g. TODO_ASSERT, TODO_LOGGING, TODO_TEARING) and listed in TODOS.md.
- Project sources live in `<Project>/src/`. Third-party code goes in ThirdParty/ as a git submodule with its premake script beside it (`ThirdParty/<name>.lua`), never inside the submodule.
- Comments: sparse and purposeful. Explain non-obvious intent or constraints only. No restating what the code does, no section-banner chatter, no "This function..." boilerplate.

## Decisions log
Record agreed decisions here (date, decision, short reason).
- 2026-11-22: Project initialised. DX12, SM 6.0 baseline. Reason: support older GPUs. Binding model is descriptor-table based on all hardware (no bindless).
- 2026-11-22: FUTURE PLAN, not implemented: higher-SM features as optional tiers detected at runtime, with raster fallbacks. First candidate: DXR inline raytracing (RayQuery, SM 6.5) for shadows. Don't build towards this until the baseline renderer exists.
- 2026-11-22: Premake5 (exe in Tools/premake/, run GenerateProjects.bat), VS2022, C++20, x64 only. Reason: developer choice.
- 2026-11-22: Core/Graphics are static libs, not DLLs. Hot reload via MSVC Edit and Continue / Hot Reload (Debug, /ZI) instead of DLL reloading. Reason: no runtime cost, allows cross-lib inlining/LTO in Release.
- 2026-11-22: Binaries build to <Project>/Build/Bin/<Config>, Game exe + pdb copied post-build to bin/<Config>, which is the debug working dir.
- 2026-11-22: `DEBUG`/`RELEASE` always defined as 0/1 for `#if`; C4668 is an error (external `<...>` headers exempt, ThirdParty projects exempt).
- 2026-11-22: Windowing via GLFW 3.4 (`GLFW_NO_API`, HWND via `glfwGetWin32Window`). Reason: small, easy to build with premake. SDL3 considered and rejected for now.
- 2026-11-22: DirectX-Headers submodule for d3dx12.h and newer d3d12.h (header-only, no premake project). Linked system libs: d3d12, dxgi, dxguid (on Game). Reason: official source; NuGet/vcpkg/Agility SDK not needed at SM 6.0.

## Open questions
- Minimum feature level: code uses 12_0 (guarantees resource binding tier 2). Suggested, not formally confirmed by the developer.
- Error handling approach (HRESULT checks, exceptions, asserts)? Currently a temporary `CHECK_HR` macro.
- Logging design (see TODOS.md).
- Remaining third-party deps (D3D12MA, DirectXTex, ImGui, math lib - DirectXMath is included but unused)?
- How to split Main.cpp into Core/Graphics.
- Heavy CMake-only third-party libs: build via a premake `kind "Makefile"` project that runs CMake, or prebuilt libs? (Discussed, not decided.)
- Shader compilation: offline DXC vs runtime?

## Maintaining this file
- Update it when decisions are made, structure changes, or something surprises Claude.
- Claude should suggest edits to this file when it would improve future output (e.g. a recurring correction, a new convention). Propose the edit; don't silently apply it unless told to.
- Remove stale information rather than appending contradictions.
