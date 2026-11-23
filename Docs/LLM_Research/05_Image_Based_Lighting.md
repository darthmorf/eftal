# 05 Image-Based Lighting (IBL)

Status: complete. Researched 2026-11-23.

## Summary
- IBL treats an environment image (HDRI) as a light surrounding the object. Without it, PBR metals look black and everything looks flat. Biggest visual win after direct lighting.
- Pipeline:
  1. Load HDR equirectangular panorama (`.hdr`, Radiance RGBE; Poly Haven, see 03) -> convert to cubemap (compute shader or pixel shader per face).
  2. **Diffuse**: irradiance = cosine-convolved environment. Store as a tiny cubemap (32x32) OR as **9 spherical harmonic (SH) coefficients** (Ramamoorthi & Hanrahan 2001: Lambert is a low-pass filter, so 2 SH bands = 9 RGB coefficients are enough). SH is cheaper and easy to pass in a constant buffer.
  3. **Specular**: **split-sum approximation** (Karis 2013). The integral is split into:
     - prefiltered environment cubemap: one mip per roughness level, convolved with GGX via importance sampling (sample the GGX lobe, N = V = R assumption);
     - **BRDF LUT**: 2D texture (n.v, roughness) -> (scale, bias) for F0. Computed once, independent of environment (R16G16_FLOAT, 128-512^2).
     - Shader: `specular = prefiltered.SampleLevel(R, roughness * maxMip) * (F0 * lut.x + lut.y)`.
  4. Combine with direct lighting; multiply by AO texture (and later SSAO/RTAO).
- Filtering tricks: "filtered importance sampling" (sample from a lower mip based on PDF) removes fireflies with few samples (GPU Gems 3 ch. 20 idea; used by Khronos IBL Sampler). Multiple-scattering energy compensation via the same LUT (Fdez-Aguera 2019; bruop write-up) is an easy extra.
- Local reflection probes (cubemaps captured in-scene, parallax-corrected) are the later step for interiors; RT reflections (08) are the high tier.
- Can be precomputed offline (Khronos glTF-IBL-Sampler, cmgen in Filament) and loaded as DDS (BC6H), or generated at load time with compute shaders. Doing it in compute is a good DX12 learning exercise (UAVs, mips, dispatch).

## Sources (easiest first)
1. **LearnOpenGL - IBL: Diffuse irradiance** https://learnopengl.com/PBR/IBL/Diffuse-irradiance and **Specular IBL** https://learnopengl.com/PBR/IBL/Specular-IBL. Easy. Complete walkthrough: equirect -> cubemap, irradiance convolution, prefilter, BRDF LUT, split sum. Uses render-to-cubemap in GL; port to compute in DX12.
2. **Karis - Real Shading in UE4** (link in 01) "Image Based Lighting" section. Easy-medium. Origin of the split-sum approximation; 2 pages; includes importance-sampling HLSL-like code.
3. **Bruno Opsenica - Image Based Lighting with Multiple Scattering** (2019) https://bruop.github.io/ibl (verified). Medium. Compute shaders (bgfx GLSL, easy to port to HLSL) for: 64x64 irradiance cubemap (hemisphere sampling, no SH), GGX prefilter with Hammersley + PDF-based source-mip trick, 128x128 RG16F BRDF LUT, Fdez-Aguera multiscatter fix. Does NOT cover equirect->cubemap (use LearnOpenGL for that). Highly recommended second read.
4. **Khronos glTF-IBL-Sampler** https://github.com/KhronosGroup/glTF-IBL-Sampler. Tool + reference code: panorama/cubemap in, filtered cubemap + BRDF LUT out. Use as offline tool or port its GGX filter to HLSL. Matches the glTF Sample Viewer, so results are directly comparable.
5. **Ramamoorthi & Hanrahan - An Efficient Representation for Irradiance Environment Maps** (SIGGRAPH 2001) https://graphics.stanford.edu/papers/envmap/. Medium. Short paper; the 9-coefficient SH irradiance formulas to copy.
6. **Peter-Pike Sloan - Stupid Spherical Harmonics (SH) Tricks** (GDC 2008) https://www.ppsloan.org/publications/StupidSH36.pdf. Medium-hard. Practical SH reference (projection, ringing, windowing, extracting dominant light). Read after (5) if using SH.
7. **DX12 open-source renderers with IBL** to read code (not tutorials): Helios https://github.com/rtarun9/Helios (C++/DX12, DXC, compute mip generation), VQEngine https://de.github.com/vilbeyli/VQEngine (SM 5.0/6.0, load-time convolution).
8. **Filament doc** "Lighting > Image based lights" https://google.github.io/filament/Filament.html. Hard, deep reference incl. SH, prefiltering details, horizon specular occlusion.

## Prerequisites
- 01, 04 (direct lighting working), 06 (compute shaders, UAVs, cubemaps, mip SRV/UAV per subresource), 07 (HDR + tonemapping - IBL is meaningless without HDR).

## Feasibility & pitfalls
- Difficulty: M overall. BRDF LUT S, SH irradiance S, prefiltered cubemap M, equirect->cubemap S.
- Pitfalls: sampling the prefiltered map with `Sample` instead of `SampleLevel` (wrong mip); visible seams at cube edges (enable seamless cube filtering - automatic in D3D11+); bright-pixel fireflies in prefilter (use filtered importance sampling or clamp); LUT generated with wrong G term (IBL uses k = alpha/2, not the direct-light remap); forgetting that `.hdr` loaders (stb_image) are C - fine for no-stdlib.
- HDR loading: stb_image (single-header C, public domain) loads `.hdr`; or convert offline to DDS BC6H with texconv.

## DX12 requirements & fallbacks
- Baseline: FL 11_0+, SM 5.1/6.0. TextureCube SRVs, RWTexture2DArray UAVs for writing faces from compute (UAV on a cube resource is a 2D array of 6 slices).
- BC6H for stored HDR cubemaps. R16G16B16A16_FLOAT for generated ones.
