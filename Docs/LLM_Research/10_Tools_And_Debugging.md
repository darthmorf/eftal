# 10 Tools and Debugging

Status: complete. Researched 2026-11-23.

## Summary
| Tool | Use for | Notes |
|---|---|---|
| **D3D12 debug layer** (already enabled in eftal Debug) | API misuse: wrong states, bad descriptors, PSO errors | First line of defence. Break on error/corruption already set via info queues. |
| **GPU-based validation (GBV)** | Errors only visible on the GPU: barrier/resource-state mistakes inside command lists, uninitialised descriptors, out-of-bounds descriptor indexing | `ID3D12Debug1::SetEnableGPUBasedValidation(TRUE)` **before** device creation. Slow; enable via a command-line flag/toggle, not always. Output arrives after `ExecuteCommandLists` completes. `ID3D12Debug3::SetGPUBasedValidationFlags` can disable state tracking to reduce cost. |
| **DRED** (Device Removed Extended Data) | GPU hangs / TDR / device removed: which command was executing, page faults | Enable via `ID3D12DeviceRemovedExtendedDataSettings` before device creation (auto-breadcrumbs + page fault reporting). After `DXGI_ERROR_DEVICE_REMOVED`, query `ID3D12DeviceRemovedExtendedData`. Cheap enough to leave on in Debug. PIX markers add context (DRED 1.2). |
| **PIX on Windows** (Microsoft, free) | Frame capture, resource/texture/buffer inspection, pipeline state, **shader debugging** (step through HLSL with PDBs), timing captures, **DXR acceleration structure viewer** | The primary DX12 tool. Compile shaders with `-Zi -Fd` (external PDBs) or `-Qembed_debug` and point PIX at them. WinPixEventRuntime for `PIXBeginEvent/EndEvent` markers. Some features limited for DXR captures (shader debugging of RT shaders historically incomplete). |
| **RenderDoc** (free, open source) | Frame capture, texture/mesh viewer, pixel history | Excellent UI, supports D3D12 on FL 11_0+. Raytracing work is treated as a **black box** by design (can capture/replay a frame that uses RT, but no AS/RT shader inspection; https://renderdoc.org/docs/behind_scenes/raytracing.html) - use PIX for RT debugging. |
| **NVIDIA Nsight Graphics** / **AMD Radeon GPU Profiler (RGP)** / Intel GPA | Vendor-level profiling (occupancy, stalls, cache), Nsight also has RT inspection | Use when optimising on that vendor's GPU. Not needed early. |

- Practical habits:
  - Name every D3D12 object (`SetName`) -> names show up in debug layer messages, PIX, DRED.
  - PIX event markers around each pass (`PIXBeginEvent(cmdList, color, "Shadow pass")`).
  - Debug views in-engine (toggle to display normals, roughness, depth, AO, shadow mask, RT hit IDs, denoiser history length). Often faster than a capture.
  - Compare against reference images (glTF Sample Viewer, see 03; or a ground-truth accumulation mode, see 09).
  - Keep a "programmatic capture" hotkey via PIX's `PIXBeginCapture`/`WinPixGpuCapturer.dll` loading (optional).
- WinPixEventRuntime is a NuGet package (C API headers + DLL) - check it against the no-stdlib rule (it's header + C-style functions; fine).

## Sources (easiest first)
1. **Microsoft - Get started with PIX** https://learn.microsoft.com/en-au/windows/win32/direct3dtools/pix/pix-overview and **Analyzing frames with GPU captures** https://learn.microsoft.com/en-us/windows/win32/direct3dtools/pix/pix-gpu-captures. Easy. Official walkthrough of captures, pipeline view, shader debugging.
2. **PIX blog** https://devblogs.microsoft.com/pix/ (documentation index https://devblogs.microsoft.com/pix/documentation/; DXR support page https://devblogs.microsoft.com/pix/directx-raytracing/). Release notes + feature deep dives. Check current PIX version's DXR capabilities here.
3. **Microsoft - Using D3D12 debug layer GPU-based validation** https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-d3d12-debug-layer-gpu-based-validation and **Understanding the D3D12 debug layer** https://learn.microsoft.com/en-us/windows/win32/direct3d12/understanding-the-d3d12-debug-layer. Easy.
4. **Microsoft - Use DRED** https://learn.microsoft.com/en-us/windows/win32/direct3d12/use-dred, original blog post https://devblogs.microsoft.com/directx/dred/, spec https://microsoft.github.io/DirectX-Specs/d3d/DeviceRemovedExtendedData.html. Easy-medium. Includes copy-pasteable setup code.
5. **RenderDoc docs** https://renderdoc.org/docs (D3D12 support/limitations: https://renderdoc.org/docs/behind_scenes/d3d12_support.html). Easy.
6. **Nsight Graphics / RGP** - vendor sites; only when profiling.

## Prerequisites
- None; set up GBV toggle + DRED + object names + PIX markers early (cheap, saves hours later). Relates to eftal TODO_SYNC (timeouts to detect hangs) and TODO_ASSERT.

## Feasibility & pitfalls
- Difficulty: S. Mostly setup.
- Pitfalls: GBV and DRED must be configured before `D3D12CreateDevice`; PIX GPU captures are not portable across GPUs/drivers; PIX needs recent drivers; shader PDBs must match the compiled shader binaries (rebuild both together); captures can fail if the app is already making invalid API calls - fix debug layer errors first.

## DX12 requirements & fallbacks
- GBV: Graphics Tools optional Windows feature (installed with the debug layer). DRED: Windows 10 1809+ (1.1), 1903+ (1.2).
