# 07 Post-Processing (HDR, exposure, tonemapping, bloom, TAA)

Status: complete. Researched 2026-11-23.

## Summary
- Order of a typical post chain: HDR scene colour -> (TAA, in HDR, before tonemap) -> exposure -> bloom (added in HDR) -> tonemap -> (colour grading LUT) -> sRGB encode -> UI on top -> present.
- **Exposure**: scale HDR values before tonemapping. Start with a manual exposure value (EV slider). Auto-exposure: build a log-luminance histogram in a compute shader (atomics on uint bins), compute average (ignoring darkest/brightest percentiles), smooth over time (eye adaptation). Physical camera EV100 model in Frostbite/Filament docs if using real light units.
- **Tonemapping**: maps HDR -> [0,1].
  - Reinhard `x/(1+x)`: simple, washed out; apply to luminance rather than per channel to keep hue.
  - Hable/Uncharted 2 filmic: classic, tunable.
  - ACES (Narkowicz 2016 fit: `(x(2.51x+0.03))/(x(2.43x+0.59)+0.14)`): popular, punchy contrast, but per-channel fit skews hue and over-saturates bright colours (bright blue -> purple, etc.).
  - AgX (Troy Sobotka, Blender 4.0 default): better handling of saturated highlights (goes towards white naturally). Real-time = log encode + polynomial sigmoid fit + inset/outset matrices. Several ports (Godot, three.js) to copy from.
  - Also worth knowing: Khronos PBR Neutral (for product/"true colour" viewing, matches the glTF sample viewer option) and Tony McMapface (LUT-based) - search by name.
  - Recommendation: implement Reinhard + ACES fit first (5 minutes), then AgX. Make it switchable at runtime for comparison.
- **Bloom**: "physically based" bloom (Jimenez, CoD: AW 2014): no brightness threshold; progressive downsample chain (13-tap filter, Karis average on first mip to kill fireflies) then upsample with 3x3 tent and accumulate; blend small amount (~4%) into HDR scene. Compute or pixel shaders.
- **Anti-aliasing**:
  - MSAA: works with forward rendering only, simple in DX12 (MSAA render target + `ResolveSubresource`), doesn't fix shader aliasing (specular shimmer).
  - FXAA/SMAA: post-process, cheap, okay quality.
  - **TAA**: jitter projection by sub-pixel offsets (Halton sequence) per frame, reproject history with motion vectors, clamp/clip history colour to the current pixel's neighbourhood min/max (or variance) to reject ghosting, blend ~10% current. Needed anyway for RT denoising and stochastic effects (08, 09). Requires motion vectors (per-object velocity buffer) and stable depth.
  - Upscalers (FSR 2/3 open source, DLSS, XeSS) are drop-in "TAA+" later.

## Sources (easiest first)
1. **LearnOpenGL - HDR, Bloom** (Advanced Lighting section, https://learnopengl.com) and **Physically Based Bloom guest article (2022)** https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom. Easy. Clear intros; the PB bloom article is exactly the CoD method.
2. **Bruno Opsenica - Automatic Exposure Using a Luminance Histogram** https://bruop.github.io/exposure and **Tone Mapping** https://bruop.github.io/tonemapping. Easy-medium. Two-part series; compute histogram code + tonemapper comparison (Reinhard, Hable, ACES) with images.
3. **Matt Pettineo (MJP) - A Closer Look at Tone Mapping** https://therealmjp.github.io/posts/a-closer-look-at-tone-mapping and **Average luminance calculation using a compute shader** https://therealmjp.github.io/posts/average-luminance-compute-shader. Medium, D3D-focused.
4. **Krzysztof Narkowicz - ACES Filmic Tone Mapping Curve** (2016, verified) https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/. Easy, short, CC0/MIT HLSL: `saturate((x*(2.51x+0.03))/(x*(2.43x+0.59)+0.14))`. The fit is pre-exposed (input 1 -> ~0.8); multiply input by 0.6 for the original ACES curve. Input is linear, exposure before, gamma after. Over-saturates brights; links Stephen Hill's more accurate fit. He also wrote a classic short "Automatic Exposure" post on the same blog.
4b. **Alex Tardif - Histogram luminance / auto-exposure** https://alextardif.com/HistogramLuminance.html (the post Bruno Opsenica's exposure article is based on). Medium, HLSL compute.
5. **AgX**: three.js / pmndrs postprocessing tone mapping (reference implementations) https://pmndrs.github.io/postprocessing/manual/demos/color-grading/tone-mapping; GLSL tonemap collection https://dmnsgn.github.io/glsl-tone-map; community gist with polynomial fit https://gist.github.com/Sam-Izdat/c1f7ea42a778a10754f1aa167ebf7f35; Godot discussion of implementation trade-offs https://github.com/godotengine/godot-proposals/discussions/7545; "Reverse Engineering Blender's AgX" https://thecodecofficial.github.io/Blog/AgES. Medium.
6. **Jorge Jimenez - Next Generation Post Processing in Call of Duty: Advanced Warfare** (SIGGRAPH 2014) https://www.iryoku.com/?p=1707. Medium-hard slides; bloom + motion blur + DOF origin material.
7. **TAA**:
   - **Emilio Lopez - Temporal AA and the Quest for the Holy Trail** (2022, updated 2025, verified) https://www.elopezr.com/temporal-aa-and-the-quest-for-the-holy-trail/. Medium. Best beginner tutorial: builds TAA step by step (jittered projection, accumulation, reprojection from depth, disocclusion, 3x3 min/max colour clamp, depth/stencil/velocity rejection, motion vectors with jitter removed, flicker/luma weighting, Catmull-Rom history sampling, texture blur/negative mip bias, velocity dilation, transparency, camera cuts) with HLSL snippets. Uses MJP's MSAAFilter demo (https://github.com/TheRealMJP/MSAAFilter) for examples. Covers clamp only; variance clipping/YCoCg via linked Playdead/UE4 talks.
   - Alex Tardif - TAA https://alextardif.com/TAA.html and antialiasing overview https://alextardif.com/Antialiasing.html. Medium, practical.
   - DKNazar notes https://dknazar.github.io/taa. Easy-medium summary.
   - Brian Karis - High Quality Temporal Supersampling (SIGGRAPH 2014 Advances, course page https://advances.realtimerendering.com/s2014/). Medium. The UE4 TAA that defined modern TAA (neighbourhood clipping, YCoCg).
   - Yang, Liu, Salvi - A Survey of Temporal Antialiasing Techniques (2020) https://research.nvidia.com/labs/rtr/publication/yang2020survey. Hard, comprehensive reference.
   - Marco Salvi - An Excursion in Temporal Supersampling (GDC 2016) https://developer.download.nvidia.com/gameworks/events/GDC2016/msalvi_temporal_supersampling.pdf. Medium: variance clipping.

## Prerequisites
- 06 (HDR render targets, compute shaders, fullscreen passes). TAA needs motion vectors (extra render target) and camera jitter.

## Feasibility & pitfalls
- Difficulty: tonemap S, manual exposure S, auto-exposure M, bloom M, TAA M-L (simple version M, artifact-free L).
- Pitfalls: tonemapping in sRGB space (do it on linear values, then encode); double sRGB encoding (SRGB RTV + manual pow); applying bloom after tonemap; TAA ghosting with no history rejection; forgetting to remove jitter for UI/debug drawing; un-jittered vs jittered matrices for motion vectors.
- Order to implement: tonemap + manual exposure (with PBR, day 1) -> bloom -> auto-exposure -> TAA (when RT or SSR/SSAO noise needs it).

## DX12 requirements & fallbacks
- All baseline SM 6.0. Histogram uses `InterlockedAdd` on groupshared/UAV uints (SM 5.0+). MSAA support for R16G16B16A16_FLOAT 4x is universal on FL 11_0+.
- HDR display output (scRGB / HDR10 swap chain) is a separate optional feature; tonemap differently for HDR displays. Skip initially.
