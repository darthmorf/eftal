# 09 Denoising and Temporal Accumulation

Status: complete. Researched 2026-11-23.

## Summary
- Real-time RT budgets allow ~0.5-1 ray per pixel per effect, so stochastic effects (soft shadows, AO, glossy reflections, GI) are noisy. Denoising = **temporal accumulation** (reuse previous frames) + **spatial filtering** (blur guided by depth/normals).
- **Temporal accumulation**:
  - Reproject: for each pixel, find where it was last frame (motion vectors / previous view-projection with depth), fetch history.
  - Validate history: reject if depth/normal/object ID differ too much (disocclusion) -> reset sample count.
  - Blend: `history = lerp(history, current, 1/(1+N))` with N capped (e.g. 16-32) - gives a running average that converges, cap keeps it responsive. Store N per pixel.
  - Ghosting/lag: clamp history to the current frame's neighbourhood min/max or mean +/- k*stddev (same trick as TAA, 07).
  - Static camera + static scene + infinite N = reference-quality accumulation (great debug mode: "ground truth" path-traced-like image).
- **Spatial filtering**: edge-aware blur that doesn't cross geometric edges. Classic: **edge-avoiding A-Trous wavelet filter** (Dammertz et al. 2010): 5x5 kernel applied 3-5 times with step size 1, 2, 4, 8, 16; weights from depth, normal and luminance differences. Cheap, simple, effective.
- **SVGF** (Schied et al. 2017) = temporal accumulation + per-pixel variance estimate + variance-guided A-Trous. The reference "first real denoiser" to implement; well understood; ~10 ms at 1080p on 2017 hardware for full GI (much less for shadows/AO). A-SVGF adds faster reaction to change.
- **Effect-specific shortcuts**:
  - Shadows: denoise the shadow *visibility* mask only (scalar), not lit colour. Very effective. Penumbra-size-aware blur (estimate from blocker distance).
  - AO: scalar too; short rays -> low noise, a simple temporal + bilateral blur usually suffices.
  - Reflections: harder (view-dependent); denoise per roughness, mirror surfaces need none, rough ones can just use IBL.
  - Demodulate albedo before filtering (filter lighting only, multiply albedo back) to keep texture detail.
- **Libraries vs own**:
  - **AMD FidelityFX Denoiser** (MIT, open source, DX12/Vulkan): shadow denoiser (1 jittered ray/pp shadow mask, tile classification) and reflection denoiser. Plus samples "Hybrid Shadows", "Hybrid Reflections". Readable HLSL - good to learn from or integrate.
  - **NVIDIA NRD** (ReBLUR, ReLAX, SIGMA shadows): best quality, but gated behind the NVIDIA RTX SDK licence agreement and complex to integrate. Not recommended for a learning project.
  - Recommendation: write own temporal accumulation + A-Trous (learning value high, effort M), use FidelityFX shadow denoiser as reference or fallback.

## Sources (easiest first)
1. **SEED - Ray Tracing Gems ch. 25 "Hybrid Rendering for Real-Time Ray Tracing"** (link in 08). Medium. Practical shadow/AO/reflection filtering in a shipped-quality demo, with pseudocode.
2. **AMD FidelityFX Denoiser** https://gpuopen.com/fidelityfx-denoiser/, docs https://gpuopen.com/manuals/fidelityfx_sdk/techniques/denoiser/. Medium. Docs explain each pass (classification, temporal reprojection, spatial filter); MIT HLSL source in the FidelityFX SDK.
3. **Schied et al. - SVGF** (HPG 2017, best paper) https://cg.ivd.kit.edu/publications/2017/svgf/svgf_preprint.pdf, project page https://research.nvidia.com/labs/rtr/publication/schied2017spatiotemporal. Medium-hard but very readable for a paper; algorithm section maps directly to compute passes.
4. **Dmitry Zhdan (NVIDIA) - Fast Denoising with Self-Stabilizing Recurrent Blurs** (GTC 2020, ReBLUR) https://developer.download.nvidia.com/video/gputechconf/gtc/2020/presentations/s22699-fast-denoising-with-self-stabilizing-recurrent-blurs.pdf. Medium-hard. Good on accumulation maths (1/(1+N)), history clamping, practical trade-offs.
5. **Survey of real-time ray tracing denoising** (arXiv 2023) https://www.arxiv.org/pdf/2312.06827. Hard, overview of the field.
6. **NVIDIA NRD** https://developer.nvidia.com/nrd, GitHub (access via RTX SDK licence) https://github.com/NVIDIA-RTX/NRD-Sample. Reference only.
- Also: Dammertz et al. "Edge-Avoiding A-Trous Wavelet Transform for fast Global Illumination Filtering" (HPG 2010) - short original A-Trous paper (search by title).

## Prerequisites
- 07 (motion vectors, TAA concepts), 08 (something noisy to denoise), 06 (compute passes, ping-pong textures, history buffers per frame).

## Feasibility & pitfalls
- Difficulty: temporal accumulation S-M, A-Trous S-M, full SVGF M-L, integrating FidelityFX M.
- Pitfalls: motion vectors wrong/missing for moving objects -> smearing; not resetting history on disocclusion; blurring across normals/depth edges (light leaking); filtering colour with albedo baked in (blurs textures); denoising in non-linear space; history buffers must be per-frame ping-pong, not read+write same texture.
- Debug views are essential: show N (history length), variance, rejected pixels.

## DX12 requirements & fallbacks
- Pure compute, SM 6.0 is enough for the denoiser itself (RT inputs come from the SM 6.5 tier). FidelityFX requires its own SDK build; check SM requirement of the version used.
