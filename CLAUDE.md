# CLAUDE.md

Persistent context for Claude. Not human-facing documentation. Keep it short and current.

## Project
- Name: eftal
- C++ DX12 renderer / engine / game. Target: Shader Model 6.0 (HLSL compiled with DXC).
- Status: skeleton only. Premake workspace with Game (exe), Core and Graphics (static libs), ThirdParty/ (empty).
- Purpose: a learning project. The developer wants to understand and own every line. Code quality and understanding matter more than speed of delivery.

## Environment
- Repo lives on Windows at this directory; Claude runs in WSL2 Ubuntu via `claude.bat`.
- DX12 builds/runs on Windows, not in WSL. Building or running from here would go through Windows interop (`cmd.exe`, `cl.exe`, `cmake.exe`, etc.) - not set up yet. Ask before assuming a toolchain.
- Default git branch is `master` (no commits yet).

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
- Until then, record decisions below as they are made rather than inventing conventions.
- Comments: sparse and purposeful. Explain non-obvious intent or constraints only. No restating what the code does, no section-banner chatter, no "This function..." boilerplate.

## Decisions log
Record agreed decisions here (date, decision, short reason).
- 2026-11-22: Project initialised. DX12, SM 6.0.
- 2026-11-22: Premake5 (exe in Tools/premake/, run GenerateProjects.bat), VS2022, C++20, x64 only. Reason: developer choice.
- 2026-11-22: Core/Graphics are static libs, not DLLs. Hot reload via MSVC Edit and Continue / Hot Reload (Debug, /ZI) instead of DLL reloading. Reason: no runtime cost, allows cross-lib inlining/LTO in Release.
- 2026-11-22: Binaries build to <Project>/Build/Bin/<Config>, Game exe + pdb copied post-build to bin/<Config>, which is the debug working dir.

## Open questions
- Windowing (raw Win32 / SDL / GLFW)?
- Error handling approach (HRESULT checks, exceptions, asserts)?
- Third-party deps (DirectX-Headers, D3D12MA, DirectXTex, ImGui, math lib)?
- Shader compilation: offline DXC vs runtime?

## Maintaining this file
- Update it when decisions are made, structure changes, or something surprises Claude.
- Claude should suggest edits to this file when it would improve future output (e.g. a recurring correction, a new convention). Propose the edit; don't silently apply it unless told to.
- Remove stale information rather than appending contradictions.
