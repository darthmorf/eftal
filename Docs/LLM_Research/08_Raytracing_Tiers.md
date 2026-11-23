# 08 Raytracing as Optional Runtime-Detected Tiers

Status: complete. Researched 2026-11-23.

## Summary
- **Two DXR flavours**:
  - **DXR 1.0 (Tier 1.0) "pipeline" raytracing**: `DispatchRays` + raytracing pipeline state object (RTPSO) + shader tables (raygen, miss, closest-hit, any-hit, intersection shaders). Flexible (per-material hit shaders, recursion), but lots of setup (state objects, shader identifiers, shader-table layout/alignment, local root signatures).
  - **DXR 1.1 (Tier 1.1) inline raytracing**: `RayQuery<FLAGS>` object + `TraceRayInline()` + `Proceed()` loop inside **any** shader stage (compute, pixel). No shader tables, no RTPSO. Microsoft recommends it for "simple" cases like shadows and AO; it is also commonly used for reflections that just fetch material data from bindless/table buffers.
  - **Recommendation for a solo dev: inline ray queries (Tier 1.1) only.** Much less API surface, fits into existing compute/pixel passes, matches the "optional tier" design (same shaders, extra `#if RT_TIER` path). Skip DXR 1.0 pipelines unless doing a full path tracer.
- **Acceleration structures** (shared by both flavours):
  - **BLAS** (bottom level): per mesh, built from vertex/index buffers. Build once for static meshes (`PREFER_FAST_TRACE` + `ALLOW_COMPACTION`, then compact). Skinned/deforming meshes: `ALLOW_UPDATE` + refit each frame.
  - **TLAS** (top level): instances (transform + BLAS address + instance mask/ID). Rebuild every frame (cheap for hundreds/thousands of instances).
  - Builds go through `BuildRaytracingAccelerationStructure` on a command list; query sizes with `GetRaytracingAccelerationStructurePrebuildInfo`; scratch + result buffers in default heap, UAV state `D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE`; UAV barrier between BLAS builds and TLAS build.
  - TLAS bound as SRV (`RaytracingAccelerationStructure` in HLSL) via root SRV or descriptor.
- **Feature detection**: `CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5)` -> `RaytracingTier` (NOT_SUPPORTED / 1_0 / 1_1 / 1_2). Inline needs Tier 1.1 and shaders compiled for **SM 6.5** (`cs_6_5` / `ps_6_5`). Also check `D3D12_FEATURE_SHADER_MODEL` for 6.5. Needs `ID3D12Device5` and `ID3D12GraphicsCommandList4`.
- **Agility SDK**: NOT needed for DXR 1.1 / SM 6.5 on Windows 10 2004+ with current drivers. Needed for SM 6.6+ (bindless `ResourceDescriptorHeap`), DXR 1.2 (opacity micromaps, shader execution reordering; SM 6.9, Agility SDK 1.619+). Keep 1.1 as the RT tier; consider a later "Agility" tier.
- **Shipping two shader sets**: compile baseline shaders as `*_6_0` and RT variants as `*_6_5` (DXC define e.g. `-D RT_ENABLED=1`). Pick at runtime from the detected tier.
- **Effects, cheapest/most valuable first**:
  1. **RT shadows** (directional/sun first): one ray per pixel towards the light from the depth-reconstructed world position; `RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER`; soft shadows by jittering the direction within a cone (light angular size) + denoising (09). Fallback: CSM (04). Difficulty M.
  2. **RTAO**: a few short cosine-distributed rays per pixel, any-hit = occluded; denoise. Fallback: SSAO / material AO. Difficulty M.
  3. **RT reflections**: reflect view ray (glossy: sample GGX lobe), trace, at hit fetch material + do simple lighting (can reuse shadows, sample IBL for misses); often half-res + spatiotemporal reconstruction; fall back to prefiltered IBL for rough surfaces (roughness > ~0.5). Needs access to per-instance vertex/index/material data at the hit (instance ID -> table of buffer descriptors). Fallback: SSR + IBL (05). Difficulty L.
  4. **GI** (advanced, flag as stretch goals): **DDGI** (irradiance probe grid updated with rays each frame, ~fixed cost, robust, fits a solo dev best); **ReSTIR** (DI/GI resampling, state-of-the-art for many lights, complex). Difficulty L-XL.
- **Hybrid rendering** (SEED PICA PICA): rasterise primary visibility, ray trace only secondary effects, each effect independently swappable between raster and RT. Exactly the tier model planned for eftal.
- **Screen-space reflections (SSR) fallback**: ray march the depth buffer from the G-buffer (linear DDA, McGuire & Mara 2014, or Hi-Z traversal, Uludag GPU Pro 5); fade at screen edges and fall back to IBL. Difficulty M.

## Sources (easiest first)
1. **NVIDIA - DX12 Raytracing tutorial Part 1 & 2** (2018) https://developer.nvidia.com/rtx/raytracing/dxr/DX12-Raytracing-tutorial-Part-1, https://developer.nvidia.com/rtx/raytracing/dxr/DX12-Raytracing-tutorial-Part-2 + extras (e.g. simple lighting/shadow ray https://developer.nvidia.com/rtx/raytracing/dxr/dx12-raytracing-tutorial/extra/dxr_tutorial_extra2_simple_lighting). Easy-medium (verified live; Lefrancois & Gautron, ~2018). Adds DXR to Microsoft's HelloTriangle sample (Win10 1809+), so it lines up with the developer's background. Part 1 = feature check (`TIER_1_0`, `ID3D12Device5`, `ID3D12GraphicsCommandList4`) + BLAS/TLAS; Part 2 = pipeline + shader binding table. Uses DXR 1.0 pipelines (not inline) and requires NVIDIA's `nv_helpers_dx12` helper classes (https://developer.nvidia.com/rtx/raytracing/dxr/dx12-raytracing-tutorial/dxr_tutorial_helpers) - read what the helpers do rather than treating them as a black box. The AS build code and feature checks transfer directly to the inline path.
2. **Adam Marrs - IntroToDXR** https://github.com/acmarrs/IntroToDXR. Easy-medium. Minimal single-project DXR sample (from the Wyman/Marrs course). Good code to read alongside (1).
3. **Microsoft DirectX blog - DXR 1.1** (Amar Patel, Nov 2019, verified) https://devblogs.microsoft.com/directx/dxr-1-1/. Easy. Explains inline raytracing with a pixel-shader `RayQuery` example (`TraceRayInline`, `Proceed`, `CommittedStatus`). Guidance: many complex shaders -> pipeline RT; simple shading/few shaders (shadows, compute, simple recursion) -> inline; "the dividing line isn't obvious", profile real content. Needs SM 6.5; no new hardware (Tier 1.0 GPUs get 1.1 via drivers).
4. **DXR functional spec** https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html. Medium-hard reference (v1.34, 2025). Read the "Inline raytracing", "RayQuery", "Acceleration structure" and "Tier" sections when implementing.
5. **Inline RT examples** (non-DX12-native but direct HLSL): Unity inline RT shadows compute shader https://github.com/INedelcu/InlineRayTracingShadows (HLSL RayQuery soft shadows, easy to read); Vulkan tutorial "Ray query shadows" https://docs.vulkan.org/tutorial/latest/courses/18_Ray_tracing/03_Ray_query_shadows.html (concepts/flags identical); NVIDIA vk ray query sample https://nvpro-samples.github.io/vk_raytracing_tutorial_KHR/samples/16-ray-query/.
6. **Ray Tracing Gems** (2019, free, CC BY-NC-ND) https://link.springer.com/book/10.1007/978-1-4842-4427-2 (also https://www.realtimerendering.com/raytracinggems/rtg/). Medium. Key chapters: 3 "Introduction to DirectX Raytracing" (Wyman et al.), 13 "Ray Traced Shadows: Maintaining Real-Time Frame Rates", 25 "Hybrid Rendering for Real-Time Ray Tracing" (SEED PICA PICA; also free from EA https://media.contentapi.ea.com/content/dam/ea/seed/presentations/2019-ray-tracing-gems-chapter-25-barre-brisebois-et-al.pdf). Chapter 6 "A Fast and Robust Method for Avoiding Self-Intersection" (ray offset) is a must-read before any RT effect.
7. **Ray Tracing Gems II** (2021, free) https://developer.nvidia.com/ray-tracing-gems-ii (PDF via Apress; code https://github.com/Apress/Ray-Tracing-Gems-II). Medium-hard. Pick chapters as needed (reflections, denoising, ReSTIR intro chapters).
8. **Intel - Arc Graphics Developer Guide for Real-Time Ray Tracing in Games** https://www.intel.com/content/www/us/en/developer/articles/guide/real-time-ray-tracing-in-games.html. Medium. Vendor-neutral-ish best practices: AS build flags, inline vs pipeline performance, ray flags.
9. **SEED - Hybrid rendering / PICA PICA talks** https://ea.com/seed/news/hybrid-rendering-for-real-time-ray-tracing, SIGGRAPH 2018 "Full Rays Ahead" deck https://media.contentapi.ea.com/content/dam/ea/seed/presentations/siggraph2018-full-rays-ahead-from-raster-to-real-time-raytracing.pdf. Medium. Architecture inspiration for raster+RT tiers.
10. **Wyman et al. - Introduction to DirectX Raytracing** (SIGGRAPH 2018 course, verified) https://intro-to-dxr.cwyman.org/. Medium. Slide PDFs, video (https://www.youtube.com/watch?v=Q1cuuepVNoY), and incremental shader tutorials with a walkthrough http://cwyman.org/code/dxrTutors/dxr_tutors.md.html (BSD). Tutorials sit on a helper framework (from memory: a simplified Falcor - check), so they teach RT shader concepts more than raw D3D12 setup. Also NVIDIA Research page https://research.nvidia.com/labs/rtr/publication/wyman2019introduction.
11. **SSR**: McGuire & Mara - Efficient GPU Screen-Space Ray Tracing (JCGT 2014) https://jcgt.org/published/0003/04/04/paper.pdf (Medium, includes code); Hi-Z: Uludag, GPU Pro 5 (book, not free) + gamedev.net implementation thread https://gamedev.net/forums/topic/658702-help-with-gpu-pro-5-hi-z-screen-space-reflections.
11b. **SSAO (RTAO fallback)**: LearnOpenGL SSAO https://learnopengl.com/Advanced-Lighting/SSAO (Easy, hemisphere sampling, based on John Chapman's tutorial); Intel XeGTAO https://github.com/GameTechDev/XeGTAO (Medium, MIT, DX/HLSL implementation of Jimenez 2016 Ground Truth AO, validated against a ray-traced reference - good production-quality option and drop-in compute shaders).
12. **DDGI**: Majercik et al. JCGT 2019 https://research.nvidia.com/publication/2019-05_dynamic-diffuse-global-illumination-ray-traced-irradiance-fields (Hard, has reference code); NVIDIA "An Engineer's Guide to Integrating DDGI" https://developer.nvidia.com/blog/an-engineers-guide-to-integrating-ddgi (Medium, practical).
13. **ReSTIR**: A Gentle Introduction to ReSTIR (SIGGRAPH 2023 course, Wyman et al., verified) https://intro-to-restir.cwyman.org/ - course notes PDF (updated 2024), per-talk slides, free ACM video via the SIGGRAPH 2023 courses page; no code. Hard. Only once everything else works.
- Background if ray tracing itself is new: "Ray Tracing in One Weekend" (Peter Shirley, free online book series) - CPU, builds intuition for rays, sampling, materials (search by name).

## Prerequisites
- 06 (solid resource/barrier/descriptor handling), 01 (BRDF for shading hits), 09 (denoising) for anything stochastic, 07 (TAA/motion vectors) for temporal accumulation.
- G-buffer-like data (depth, normals, roughness) available to compute passes.

## Feasibility & pitfalls
- Difficulty: AS building M; RT hard shadows S-M (after AS); soft shadows/RTAO M + denoiser; reflections L; DDGI L; ReSTIR XL.
- Suggested order: feature detection + device5/cmdlist4 -> BLAS/TLAS for static scene -> debug view (trace primary rays in compute, output hit instance ID) -> RT hard shadows (sun) -> soft shadows + denoise -> RTAO -> reflections -> GI.
- Pitfalls: self-intersection (offset ray origin along normal - RTG ch. 6); forgetting UAV barriers after AS builds; TLAS instance transforms are 3x4 row-major; BLAS must stay alive while referenced; resource states for AS buffers are fixed (`RAYTRACING_ACCELERATION_STRUCTURE`); alpha-tested geometry needs any-hit logic (`CANDIDATE_NON_OPAQUE_TRIANGLE` handling in the `Proceed()` loop) - mark opaque geometry `D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE`; memory: scratch buffers can be large - reuse one; reflections need material/vertex access at hit points without bindless -> use one big descriptor table of all mesh buffers indexed by `InstanceID()`/`GeometryIndex()` (unbounded table, resource binding tier 2+).
- PIX supports AS visualisation - use it (10).

## DX12 requirements & fallbacks
| Feature | Requirement | Fallback |
|---|---|---|
| Any DXR | `RaytracingTier >= 1_0`, `ID3D12Device5`, Win10 1809+ | raster |
| Inline RayQuery | Tier 1.1, SM 6.5 shaders, Win10 2004+ | raster |
| DXR 1.2 (OMM, SER) | Agility SDK 1.619+, SM 6.9 | Tier 1.1 path |
| RT shadows | Tier 1.1 | CSM + PCF (04) |
| RTAO | Tier 1.1 | SSAO / baked AO |
| RT reflections | Tier 1.1 | SSR + IBL (05) |
| DDGI / RT GI | Tier 1.1 | IBL / SH ambient / baked probes |
- Hardware: Tier 1.1 on NVIDIA RTX 20xx+, AMD RX 6000+, Intel Arc. (GTX 10xx had a slow driver-emulated Tier 1.0 path; treat as unsupported.)
