# 04 Direct Lighting and Shadows

Status: complete. Researched 2026-11-23.

## Summary
- **Punctual lights**: directional (sun), point, spot. Radiance contribution = BRDF * light colour * intensity * attenuation * n.l.
  - Physical attenuation = inverse square `1/d^2`. Games add a **window function** so the light reaches exactly 0 at a radius (enables culling): UE4/Karis `saturate(1 - (d/r)^4)^2 / (d^2 + 1)`. Same idea in Filament.
  - Spot cone: smooth falloff between inner/outer angle (Filament gives a cheap formula).
  - Units (lux, lumens, candela) + physical camera exposure (Frostbite/Filament) are optional; start unitless and add exposure later (07).
- **Shadow maps**: render depth from the light, compare in the main pass.
  - Artifacts: shadow **acne** (self-shadowing) -> depth bias + slope-scaled bias (set in the PSO rasterizer state `DepthBias`/`SlopeScaledDepthBias`) and/or normal-offset; **Peter Panning** (too much bias) -> tight near/far planes; **perspective aliasing** -> cascades.
  - **Cascaded shadow maps (CSM)** for the sun: split the view frustum into 3-4 depth ranges, one orthographic shadow map each (Texture2DArray). Stabilise: snap cascade origin to texel increments + use bounding spheres to stop shimmering when the camera moves/rotates.
  - **Filtering**: hardware PCF via `SampleCmpLevelZero` + comparison sampler (free 2x2 bilinear PCF), then a wider kernel (e.g. 3x3 / Poisson disk). Soft-shadow techniques (PCSS, VSM/EVSM, MSM) are optional extras.
  - Point lights: cube shadow maps (6 renders), expensive - limit to a few. Spot: a single perspective shadow map.
- **Area lights (advanced)**: Linearly Transformed Cosines (LTC, Heitz et al. 2016) gives analytic polygon/line lights with GGX in real time using 2 small LUT textures. Nice upgrade after everything else works; not required.
- Many lights: see 06 (forward+ / clustered).

## Sources (easiest first)
1. **LearnOpenGL - Shadow Mapping** https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping and **CSM guest article (2021)** https://learnopengl.com/Guest-Articles/2021/CSM. Easy. Best introduction: acne, bias, Peter Panning, PCF, then cascades. GLSL/GL.
2. **Microsoft - Common Techniques to Improve Shadow Depth Maps** https://learn.microsoft.com/en-us/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps and companion **Cascaded Shadow Maps** article (linked from it). Easy-medium. D3D-oriented; classic, very clear explanations of aliasing types, near/far fitting, bias. Sample code was D3D11 (CascadedShadowMaps11) but concepts are identical.
3. **Matt Pettineo (MJP) - A Sampling of Shadow Techniques** (blog, 2013, updated 2023) https://therealmjp.github.io/posts/shadow-maps (verified), code https://github.com/TheRealMJP/Shadows (D3D11, prebuilt exe in releases). Medium. D3D11 sample comparing CSM stabilisation, cascade selection, PCF kernels, EVSM/MSM. Best "what actually works in practice" reference. MJP's blog is DX-centric and high quality generally.
4. **Karis - Real Shading in UE4** (see 01 for link) section on light falloff (windowed inverse square). **Filament doc** "Lighting > Direct lighting" for punctual light maths and units: https://google.github.io/filament/Filament.html.
5. **LearnOpenGL - Area Lights (LTC) guest article (2022)** https://learnopengl.com/Guest-Articles/2022/Area-Lights. Medium. Most approachable LTC explanation.
6. **Heitz et al. - LTC reference code + demos** https://github.com/selfshadow/ltc_code, slides: https://www.advances.realtimerendering.com/s2016/s2016_ltc_rnd.pdf. Hard. Use with (5) when implementing. LUT data differs from paper - follow the code. Cite the paper if used.
7. **Lauritzen - Sample Distribution Shadow Maps** (Intel, SIGGRAPH 2010 notes) https://www.intel.com/content/dam/develop/external/us/en/documents/sampledistributionshadowmaps-siggraph2010-notes-181237.pdf. Hard, optional: computes tight cascade bounds from the depth buffer.
8. **GPU Gems 3 ch. 10 - Parallel-Split Shadow Maps** https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html. Medium, older but explains split-distance schemes (log/uniform blend).

## Prerequisites
- 01 (BRDF), 06 basics (multiple passes, depth buffers, PSOs, Texture2DArray).

## Feasibility & pitfalls
- Difficulty: S (punctual lights, single shadow map), M (stable CSM + PCF), L (LTC area lights, many shadowed lights).
- Order: one directional light + one shadow map -> PCF -> CSM (2-4 cascades) -> point/spot lights -> shadow atlas if many.
- Pitfalls: bias tuning per cascade; shimmering if cascades aren't stabilised; forgetting to transition the shadow map between DEPTH_WRITE and PIXEL_SHADER_RESOURCE; reversed-Z (recommended for main camera) needs flipped comparison - keep shadow maps conventional or be consistent.
- Later RT tier replaces/augments shadow maps with ray-traced shadows (08) - keep shadowing behind a function so both paths can coexist.

## DX12 requirements & fallbacks
- All baseline: FL 11_0+, SM 5.1/6.0. Depth formats: `D32_FLOAT` (or `R32_TYPELESS` resource with D32 DSV + R32_FLOAT SRV), comparison sampler (`D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT`), static sampler in the root signature is easiest.
- Rasterizer depth bias set per PSO -> separate PSO for shadow pass.
