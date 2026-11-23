# 11 Suggested Implementation Order (solo dev)

Status: complete. Written 2026-11-23 from the research in files 01-10. A suggestion, not a plan the developer has agreed to.

Starting point: eftal = GLFW window + D3D12 clear-colour loop (3dgep Lesson 1 equivalent), SM 6.0 baseline, descriptor tables, no stdlib.
Guiding rules: each phase ends with something visible and testable; keep RT as an add-on tier, never a requirement; build debug views as you go.

## Phase 0 - Debugging foundation (S) -> 10
- Object naming, PIX markers, GBV toggle, DRED. Logging already exists (Core/Logging).

## Phase 1 - Core plumbing (M) -> 06
- 3dgep Lesson 2: vertex/index buffers, upload, root signature, PSO, depth buffer, draw a cube.
- DXC offline shader compilation in the build (premake custom step), external PDBs for PIX.
- Upload ring buffer for per-frame constants; descriptor allocator; resource state tracking helpers (3dgep Lesson 3 ideas).
- Camera (view/projection; decide handedness + reversed-Z now).

## Phase 2 - Textures and assets (M) -> 02, 03
- 3dgep Lesson 4: textures, samplers, mipmaps.
- Offline texconv -> DDS (BC7 sRGB / BC5 normals / BC4 or BC7 ORM); minimal DDS loader.
- cgltf loader: meshes + materials + node transforms. Test with Khronos `Box`, `BoxTextured`, `DamagedHelmet`.
- Validate exports with glTF Validator; compare with glTF Sample Viewer.

## Phase 3 - PBR direct lighting + HDR (S-M) -> 01, 04, 07
- HDR render target (R16G16B16A16_FLOAT) + tonemap pass (Reinhard/ACES fit) + manual exposure + sRGB output.
- Cook-Torrance GGX/Smith/Schlick + Lambert, metallic-roughness from glTF, normal mapping (tangents from file or MikkTSpace).
- Directional + point + spot lights with windowed falloff.
- Test: `MetalRoughSpheres` grid looks like the Sample Viewer.

## Phase 4 - Image-based lighting (M) -> 05
- Equirect HDRI (Poly Haven) -> cubemap (compute). SH9 irradiance. GGX prefiltered cubemap. BRDF LUT.
- Or: offline glTF-IBL-Sampler outputs first, compute versions later.
- Test: helmets under several HDRIs vs Sample Viewer.

## Phase 5 - Shadows (M) -> 04
- Single shadow map for the sun -> PCF -> stable CSM (3-4 cascades). Spot light shadows. (Point light cube shadows optional.)
- Test: Sponza (classic, then Intel 2022).

## Phase 6 - Post and screen-space effects (M) -> 07, 08 (SSR)
- Bloom (downsample/upsample chain). Auto-exposure (histogram).
- Motion vectors + TAA (needed by everything temporal later).
- Optional: SSAO, SSR (fallbacks for the RT tier).
- At this point: a complete, good-looking raster PBR renderer. Everything after this is optional tiers.

## Phase 7 - Scaling up (M, optional) -> 06
- Many lights: tiled/clustered forward+ light culling.
- D3D12MA or own placed-resource allocator. Frame graph / pass abstraction if the pass list gets unwieldy.
- Big scene performance: frustum culling, instancing. Test: Bistro.

## Phase 8 - RT tier 1: infrastructure + shadows (M) -> 08, 09, 10
- Detect `RaytracingTier >= 1_1` + SM 6.5; compile `_6_5` shader variants; fall back cleanly.
- BLAS per mesh (compacted), TLAS per frame. Debug view: primary rays in compute showing instance IDs.
- RT hard shadows (sun) replacing CSM when enabled -> soft shadows (cone jitter) -> temporal accumulation + A-Trous on the shadow mask (or FidelityFX shadow denoiser).

## Phase 9 - RT tier 2: AO and reflections (L) -> 08, 09
- RTAO (short rays, scalar denoise).
- RT reflections: instance/geometry -> buffer descriptor table to fetch hit attributes and materials; simple hit lighting (+ RT shadow ray, IBL for misses); half-res + denoise; rough surfaces fall back to IBL.

## Phase 10 - Stretch goals (L-XL)
- DDGI probe GI (best fit for a solo dev among RT GI options).
- LTC area lights. Multiscatter energy compensation. Clear coat / sheen (glTF extensions).
- SM 6.6 bindless tier (Agility SDK) to simplify material/RT binding.
- ReSTIR DI for many lights. Upscaler (FSR).

## Rough effort guide
S = an evening or two; M = one to a few weekends; L = several weeks; XL = research-project scale. Phases 0-6 are the core; a realistic solo timeline is months, not weeks - that's normal.

## Time estimate (made 2026-11-23)
Assumptions: 2-3 evenings/week at ~2-3 h (~6-7 h/week); pace based on the first sessions (window + clear loop, premake setup, logger in ~2 days of evenings); learning-first, writing own code, no-stdlib (own containers/allocators add overhead).
| Phases | Hours (rough) | Calendar at ~6-7 h/week |
|---|---|---|
| 0-1 debug foundation + core plumbing | 25-35 | 1-1.5 months |
| 2 textures + glTF/DDS pipeline | 20-30 | ~1 month |
| 3 PBR direct + HDR/tonemap | 10-15 | 2-3 weeks |
| 4 IBL | 15-20 | ~3 weeks |
| 5 shadows (CSM) | 15-20 | ~3 weeks |
| 6 bloom, auto-exposure, TAA (+SSAO/SSR optional) | 20-35 | 1-1.5 months |
| **Core raster PBR renderer (0-6)** | **~105-155** | **~4-6 months** |
| 7 scaling (clustered lights, allocator, culling) | 15-30 | 0.5-1 month |
| 8 RT infra + shadows + denoise | 25-40 | 1-1.5 months |
| 9 RTAO + RT reflections | 30-50 | 1-2 months |
| **Through RT tier (0-9)** | **~175-275** | **~7-11 months** |
| 10 stretch (DDGI, LTC, bindless, ReSTIR) | open-ended | months each for the big ones |
- Biggest risks to the estimate: engine plumbing/refactors (splitting Main.cpp, abstractions) and stdlib-replacement work, which aren't rendering features but eat evenings; debugging sync/barrier bugs; motivation dips on non-visual phases. Re-estimate after Phase 2 using actual pace.
