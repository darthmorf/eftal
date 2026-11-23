# LLM Research Library: real-time PBR + DX12 + optional raytracing

Written by Claude on 2026-11-23 as persistent memory for future sessions. Purpose: point the developer to **sources** they can learn from (not solutions), with enough summary for Claude to explain concepts and recommend reading.

How to use (for Claude): read this index, then only the relevant topic file. Every file uses the same template: Summary / Sources (easiest first) / Prerequisites / Feasibility & pitfalls / DX12 requirements & fallbacks.
Link policy: all URLs were seen in search results. Sources marked "(verified)" were opened and checked on 2026-11-23 (3dgep lessons, Karis notes, Bruno Opsenica's IBL post, MJP's shadow post, NVIDIA DXR tutorial, DXR 1.1 blog, GPU Gems 3 ch. 24, the TAA/ACES/Tardif posts, DXR/ReSTIR course sites, glTF model names). Others were judged from search summaries + prior knowledge. Big references (Filament doc, DXR spec, pbrt, PDFs) were deliberately not fetched.

## Topics
| File | Summary | Status |
|---|---|---|
| [01_PBR_Foundations.md](01_PBR_Foundations.md) | Reflectance equation, Cook-Torrance (GGX, Smith, Schlick), metal-roughness, energy conservation, linear/sRGB, HDR | complete |
| [02_Materials_And_Textures.md](02_Materials_And_Textures.md) | glTF material model, normal mapping/tangents (MikkTSpace), BCn compression, mipmaps, texconv | complete |
| [03_Test_Models_And_Asset_Pipeline.md](03_Test_Models_And_Asset_Pipeline.md) | Free test assets + licences, which model tests what, Blender import/fix/export to glTF, validation, cgltf | complete |
| [04_Direct_Lighting_And_Shadows.md](04_Direct_Lighting_And_Shadows.md) | Punctual lights + falloff, shadow maps, CSM, PCF, LTC area lights | complete |
| [05_Image_Based_Lighting.md](05_Image_Based_Lighting.md) | Split-sum IBL, SH irradiance, prefiltered cubemaps, BRDF LUT, compute generation | complete |
| [06_DX12_Renderer_Architecture.md](06_DX12_Renderer_Architecture.md) | Root signatures/descriptor tables at SM 6.0, constant/upload buffers, memory, forward vs deferred vs forward+/clustered, DXC | complete |
| [07_Post_Processing.md](07_Post_Processing.md) | Exposure/auto-exposure, tonemapping (Reinhard, ACES, AgX), bloom, MSAA/TAA | complete |
| [08_Raytracing_Tiers.md](08_Raytracing_Tiers.md) | DXR 1.0 vs 1.1 inline RayQuery, BLAS/TLAS, tier detection, SM 6.5, Agility SDK, RT shadows/AO/reflections, SSR fallback, DDGI/ReSTIR | complete |
| [09_Denoising_And_Temporal_Accumulation.md](09_Denoising_And_Temporal_Accumulation.md) | Temporal accumulation, A-Trous, SVGF, FidelityFX vs NRD | complete |
| [10_Tools_And_Debugging.md](10_Tools_And_Debugging.md) | Debug layer, GPU-based validation, DRED, PIX, RenderDoc, vendor profilers | complete |
| [11_Implementation_Roadmap.md](11_Implementation_Roadmap.md) | Suggested phase order from the current clear-colour app to RT tiers | complete |

## Top 10 sources to start with
1. LearnOpenGL PBR section (Theory, Lighting, IBL) - https://learnopengl.com/PBR/Theory (01, 05)
2. 3dgep Learning DirectX 12 lessons 2-4 - https://www.3dgep.com/learning-directx-12-2/ (and -3, -4; code https://github.com/jpvanoosten/LearningDirectX12) (06)
3. Karis, Real Shading in Unreal Engine 4 (SIGGRAPH 2013 notes) - see 01 for link (01, 05)
4. Khronos glTF 2.0 spec (Materials + Appendix B BRDF) - https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html (02)
5. Khronos glTF Sample Assets + Sample Viewer - https://github.com/KhronosGroup/glTF-Sample-Assets, https://github.khronos.org/glTF-Sample-Viewer-Release/ (03)
6. Bruno Opsenica's blog (IBL, tonemapping, auto-exposure) - https://bruop.github.io/ibl (05, 07)
7. MJP's blog (shadows, tonemapping, D3D12 memory) - https://therealmjp.github.io/posts/shadow-maps (04, 06, 07)
8. Filament PBR design doc (reference) - https://google.github.io/filament/Filament.html (01, 04, 05)
9. NVIDIA DX12 Raytracing tutorial + Microsoft DXR 1.1 blog - https://developer.nvidia.com/rtx/raytracing/dxr/DX12-Raytracing-tutorial-Part-1, https://devblogs.microsoft.com/directx/dxr-1-1/ (08)
10. Ray Tracing Gems I & II (free) - https://link.springer.com/book/10.1007/978-1-4842-4427-2, https://developer.nvidia.com/ray-tracing-gems-ii (08, 09)

## Cross-cutting notes for eftal
- No-stdlib rule: prefer C libraries (cgltf, stb_image, MikkTSpace). DirectXTex/tinygltf use the STL -> offline tools only or avoid. Check D3D12MA and FidelityFX before adopting.
- RT tier = DXR 1.1 inline RayQuery, SM 6.5, no Agility SDK needed. Bindless (SM 6.6) would be a separate optional tier.
