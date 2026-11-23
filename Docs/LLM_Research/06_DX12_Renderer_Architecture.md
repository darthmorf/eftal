# 06 DX12 Renderer Architecture (SM 6.0 baseline)

Status: complete. Researched 2026-11-23.

## Summary
- **Binding model (no bindless at the baseline)**: root signature = "function signature" for shaders. Parameters: root constants (32-bit values, fastest, small), root descriptors (CBV/SRV/UAV GPU VA, no bounds), descriptor tables (offset+count into the shader-visible heap). Samplers live in a separate heap/table; static samplers in the root signature are simplest.
  - Typical PBR layout: [0] root CBV per-draw (world matrix), [1] root CBV per-frame (camera, lights), [2] descriptor table: material SRVs (5 textures), [3] descriptor table: frame SRVs (shadow map, IBL cubemaps, BRDF LUT), static samplers.
  - Descriptor management: one big shader-visible CBV_SRV_UAV heap. Copy material descriptors from a CPU-only staging heap into a per-frame "ring" region each frame (`CopyDescriptorsSimple`), or allocate persistent contiguous ranges per material at load time (simpler, fine for a small engine).
- **Constant buffers**: 256-byte aligned. Per-frame upload-heap ring buffer: map once, write per-draw data, bind via root CBV GPU address. Must not overwrite data the GPU is still reading -> one region per frame in flight (ties into the existing per-frame fence values in Main.cpp).
- **Resource upload**: default heap (GPU-only) resources filled via an upload-heap staging buffer + `CopyBufferRegion`/`CopyTextureRegion` (`UpdateSubresources` helper in d3dx12.h), then barrier to the read state. Batch uploads at load time; separate copy queue optional.
- **Memory**: committed resources are fine to start. Later use D3D12MA (AMD GPUOpen, MIT) for placed resources/sub-allocation. Newer `D3D12_HEAP_TYPE_GPU_UPLOAD` (ReBAR, needs Agility SDK / recent Windows) - optional.
- **Render targets**: HDR scene colour (R16G16B16A16_FLOAT), depth (D32_FLOAT, consider **reversed-Z**: clear to 0, `GREATER` compare, swap near/far in projection - much better precision), then tonemap to the swap-chain back buffer (see 07).
- **Pipeline choice**:
  - **Forward**: simplest, MSAA works, transparency easy. Cost ~objects x lights. Fine for a handful of lights.
  - **Deferred**: G-buffer (albedo, normal, roughness/metal, depth) then lighting pass. Scales with many lights, decouples geometry and lighting, but more memory/bandwidth, no MSAA, transparency needs a forward pass anyway.
  - **Forward+ (tiled) / clustered**: depth prepass, compute shader bins lights into screen tiles (or 3D clusters), forward pass loops only over relevant lights. Best of both for many lights.
  - Recommendation for solo dev: **forward + depth prepass** first; add tiled/clustered light culling when light count demands it. Deferred is also reasonable but more plumbing. A G-buffer-like set of extra outputs (normals, roughness, motion vectors) will be needed anyway for SSR/SSAO/TAA/RT denoising, so a "forward with extra outputs" or thin G-buffer hybrid is common.
- **Shaders**: DXC (`dxc.exe`, ships in the Windows SDK and on GitHub). Profiles `vs_6_0`, `ps_6_0`, `cs_6_0`. Offline compile at build time (premake custom build step) to `.cso` + external PDB (`-Zi -Fd`) for PIX debugging, OR runtime compile via dxcompiler.dll for hot reload. Recommendation: offline first; runtime compile later for shader hot-reload.
- **Mipmap generation**: compute shader downsample (3dgep lesson 4 does exactly this) - or offline (02).

## Sources (easiest first)
1. **3dgep - Learning DirectX 12, Lessons 2-4** (Jeremiah van Oosten, 2018-2019, lesson 4 updated 2021). Easy-medium, DX12, the developer's current tutorial. Lesson 2 (rendering: vertex/index upload, root signatures, PSO), Lesson 3 (framework classes: UploadBuffer, DescriptorAllocator, resource state tracking), Lesson 4 (textures, samplers in root signature, compute mipmap generation). Verified URLs: https://www.3dgep.com/learning-directx-12-1/, https://www.3dgep.com/learning-directx-12-2/, https://www.3dgep.com/learning-directx-12-3/, https://www.3dgep.com/learning-directx-12-4/. Code: https://github.com/jpvanoosten/LearningDirectX12. The series stops at lesson 4 (no lesson 5). Note: lesson 4 uses DirectXTex (STL) for loading.
2. **Microsoft Learn - D3D12 programming guide**: Root Signatures https://learn.microsoft.com/windows/win32/direct3d12/root-signatures, Example Root Signatures https://learn.microsoft.com/en-us/windows/desktop/direct3d12/example-root-signatures, Descriptor Tables https://learn.microsoft.com/en-us/windows/win32/direct3d12/descriptor-tables, Resource binding flow https://learn.microsoft.com/en-us/windows/win32/direct3d12/resource-binding-flow-of-control, Memory Management https://learn.microsoft.com/en-gb/windows/win32/direct3d12/memory-management, Uploading resources https://learn.microsoft.com/en-ie/Windows/Win32/direct3d12/uploading-resources. Medium. The reference; read the binding pages when designing the root signature.
3. **3dgep - Forward vs Deferred vs Forward+ Rendering with DirectX 11** (2015) https://3dgep.com/forward-plus/ + code https://github.com/3dgep/ForwardPlus. Medium. Same author as the DX12 tutorial; long, very clear comparison with performance numbers and tiled light culling compute shader. DX11 but concepts and HLSL transfer directly.
4. **Clustered shading**: Olsson, Billeter, Assarsson - Clustered Deferred and Forward Shading (HPG 2012) https://www.cse.chalmers.se/~uffe/clustered_shading_preprint.pdf (Hard, original paper); DaveH355 clustered-shading tutorial https://github.com/DaveH355/clustered-shading (Medium, OpenGL, beginner-friendly); Angel Ortiz's Hybrid Rendering Engine (README links his excellent illustrated blog post on efficient rendering algorithms) https://github.com/angelo1211/hybridrenderingengine.
5. **Matt Pettineo (MJP) - GPU Memory Pools in D3D12** https://therealmjp.github.io/posts/gpu-memory-pool and **Ten Years of D3D12** https://therealmjp.github.io/posts/ten-years-of-d3d12. Medium-hard. How upload/default heaps actually map to hardware; retrospective of D3D12 best practices.
6. **D3D12 Memory Allocator** (AMD GPUOpen, MIT) https://gpuopen.com/d3d12-memory-allocator/, docs https://gpuopen-librariesandsdks.github.io/D3D12MemoryAllocator/html/. When committed resources become a problem. Supports custom CPU allocation callbacks. Check its STL usage against the no-stdlib rule before adopting.
7. **DXC wiki - Using dxc.exe and dxcompiler.dll** https://github.com/microsoft/DirectXShaderCompiler/wiki/Using-dxc.exe-and-dxcompiler.dll; FXC->DXC porting notes https://github.com/microsoft/DirectXShaderCompiler/wiki/Porting-shaders-from-FXC-to-DXC.
8. **Alex Tardif - Learning D3D12 from D3D11** (~2019, parts 1-3, verified) https://alextardif.com/D3D11To12P1.html (queues, fences), https://alextardif.com/D3D11To12P2.html (resources, heaps, descriptors), https://alextardif.com/D3D11To12P3.html. Medium. Concise, practical; builds its own small wrappers (borrows from MiniEngine), no D3D12Lite dependency in this series. His newer 2022 tutorial https://alextardif.com/DX12Tutorial.html is the one that uses his D3D12Lite helper. Post index: https://alextardif.com/eden.html.
9. **Chuck Walbourn - Getting Started with Direct3D 12** https://walbourn.github.io/getting-started-with-direct3d-12 and **Microsoft DirectX-Graphics-Samples** (already used by the developer). Reference.
10. **Microsoft - Important changes from D3D11 to D3D12** https://learn.microsoft.com/en-us/windows/win32/direct3d12/important-changes-from-directx-11-to-directx-12. Easy overview if anything feels unclear conceptually.
- Reversed-Z: Nathan Reed's "Depth Precision Visualized" (NVIDIA developer blog, 2015) is the standard explanation (search by title).

## Prerequisites
- Current state of the engine (clear loop with fences) -> 3dgep lessons 2-4.

## Feasibility & pitfalls
- Difficulty: M overall; this is the "engine plumbing" that everything else sits on. Worth doing carefully.
- Suggested abstractions (small, not over-engineered): upload ring buffer, descriptor allocator (linear per-frame + persistent), resource+state wrapper, PSO/root signature creation helpers, shader loading. 3dgep lesson 3 shows reasonable versions.
- Pitfalls: CBV 256-byte alignment; overwriting per-frame data still in use by GPU; missing/incorrect resource barriers (debug layer catches most; GPU-based validation catches more, see 10); descriptor heap must be set (`SetDescriptorHeaps`) before setting tables; only one CBV_SRV_UAV + one sampler shader-visible heap bound at a time; root signature 64 DWORD limit (table = 1, root descriptor = 2, constant = 1 each).
- Bindless (SM 6.6 `ResourceDescriptorHeap[]`) would simplify a lot but is above the SM 6.0 baseline - could become an optional tier like RT.

## DX12 requirements & fallbacks
- FL 12_0 guarantees resource binding tier 2 (unbounded SRV tables, enough for descriptor-table design). Everything here works at SM 6.0.
- Reversed-Z: no requirement. Float depth: universal.
