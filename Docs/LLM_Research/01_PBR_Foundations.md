# 01 PBR Foundations

Status: complete. Researched 2026-11-23.

## Summary
- PBR = shading based on physical principles so materials look right under any lighting. Three pillars: microfacet surface model, energy conservation, physically based BRDF.
- Reflectance equation (specialised rendering equation): outgoing radiance = integral over hemisphere of BRDF * incoming radiance * cos(theta). For punctual lights the integral collapses to a sum over lights.
- BRDF = diffuse + specular. Diffuse: Lambert (albedo/pi) is standard; Disney/Frostbite diffuse are optional extras.
- Specular: Cook-Torrance microfacet `D * F * G / (4 (n.l)(n.v))`.
  - D (NDF): GGX / Trowbridge-Reitz. Industry convention: alpha = roughness^2 (perceptual roughness).
  - G (geometry / masking-shadowing): Smith. UE4 uses Schlick-GGX approx with k = alpha/2 (k = (r+1)^2/8 for direct light); Frostbite uses height-correlated Smith (often written as a combined "visibility" term V = G / (4 n.l n.v)).
  - F (Fresnel): Schlick approximation `F0 + (1-F0)(1-v.h)^5`. Dielectric F0 ~0.04; metals use base colour as F0.
- Metallic-roughness workflow: baseColor, metallic (0/1 mostly), roughness, plus normal/AO/emissive. Same as glTF 2.0 (see 02).
- Energy conservation: diffuse weighted by (1 - F) * (1 - metallic). Single-scattering GGX loses energy at high roughness; "multiscatter energy compensation" (Filament, Kulla-Conty) is an optional later fix.
- Linear vs sRGB: all lighting maths in linear space. Colour textures stored sRGB -> use `_SRGB` DXGI formats so sampling linearises for free. Non-colour data (normals, roughness, metallic, AO) must be UNORM, never sRGB. Output to an sRGB backbuffer view or apply the curve in the tonemap pass.
- HDR: lighting values exceed 1.0 -> render to a float target (R16G16B16A16_FLOAT, or R11G11B10_FLOAT to save bandwidth), then tonemap to LDR (see 07).

## Sources (easiest first)
1. **LearnOpenGL - PBR Theory / Lighting** (Joey de Vries, web tutorial, OpenGL/GLSL). Easy. https://learnopengl.com/PBR/Theory then https://learnopengl.com/PBR/Lighting. The best first read: intuitive explanations of reflectance equation, microfacets, D/F/G with pictures and working shader code. GLSL -> HLSL translation is trivial; ignore the GL API parts.
2. **Brian Karis - Real Shading in Unreal Engine 4** (SIGGRAPH 2013 course notes, ~10 pages). Easy-medium. https://cgvr.informatik.uni-bremen.de/teaching/cg_literatur/Brian%20Karis%20-%20Real%20Shading%20in%20Unreal%20Engine%204,%202013%20Siggraph%20Course.pdf (Epic's official copy https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf loads, 59 pages - likely Epic's combined 2013 notes with Karis's section inside) (slides: https://neil3d.github.io/reading/assets/slides/s2013_pbs_epic_slides.pdf). The model nearly every engine copied; read "Shading Model" section and the IBL section (relevant for 05). Short and practical.
3. **Brian Karis - Specular BRDF Reference** (blog, 2013). Easy, reference. https://graphicrants.blogspot.jp/2013/08/specular-brdf-reference.html. Cheat-sheet of D/G/F variants with formulas; use when comparing options, not for learning.
4. **Jakub Boksansky - Crash Course in BRDF Implementation** (article + code, 2021, updated 2023). Medium. https://boksajak.github.io/blog/BRDF (PDF: https://boksajak.github.io/files/CrashCourseBRDF.pdf, code: https://github.com/boksajak/brdf). Written for rasterisation AND ray tracing; single-file code works in C++ and HLSL; covers importance sampling (needed later for IBL prefiltering and RT reflections). Public domain code.
5. **Google Filament - Physically Based Rendering in Filament** (engine design doc). Medium-hard. https://google.github.io/filament/Filament.html. Excellent, very thorough: sections "Standard model", "Specular BRDF", "Energy conservation", "Lighting" (light units), "Imaging pipeline". Read section by section as needed rather than front to back. GL/Vulkan engine but the doc is API-agnostic.
6. **Lagarde & de Rousiers - Moving Frostbite to PBR** (SIGGRAPH 2014 course notes, 118 pages). Medium-hard. Notes: https://cgvr.informatik.uni-bremen.de/teaching/cg_literatur/Moving%20Frostbite%20to%20Physically%20Based%20Rendering%203.0,%202014,%20Sebastien%20Lagarde.pdf, slides: https://blog.selfshadow.com/publications/s2014-shading-course/frostbite/s2014_pbs_frostbite_slides.pdf. Production-engine view: light units, physical camera/exposure, height-correlated Smith, transition pitfalls. Reference for later, not first read.
7. **Naty Hoffman - Physics and Math of Shading** (SIGGRAPH PBS course, 2013-2015 versions). Medium, some maths. 2015 slides: https://cgvr.informatik.uni-bremen.de/teaching/cg_literatur/Naty%20Hoffman%20-%20s2015_pbs_physics_math_slides.pdf, 2013 notes: https://cgvr.cs.uni-bremen.de/teaching/cg_literatur/Naty%20Hoffman%20-%20s2013_pbs_physics_math_notes.pdf. The "why" behind everything (light/matter interaction, why F0 0.04). Good once LearnOpenGL makes sense.
8. **SIGGRAPH "Physically Based Shading in Theory and Practice" course archive** (Self Shadow, yearly 2012-2025). Reference index. https://blog.selfshadow.com/publications/s2014-shading-course/ (other years same URL pattern, e.g. s2025-shading-course). Go-to place for state-of-the-art talks; mostly beyond solo-dev needs.
9. **GPU Gems 3 ch. 24 - The Importance of Being Linear** (Gritz & d'Eon, 2008, free online). Easy. https://developer.nvidia.com/gpugems/gpugems3/part-iv-image-effects/chapter-24-importance-being-linear (verified). Classic explanation: linearise textures, light in linear space, gamma-correct only at the end via sRGB textures/framebuffers.
10. **pbrt 4th edition - Physically Based Rendering: From Theory to Implementation** (Pharr, Jakob, Humphreys, free online book). Hard, offline path tracer. https://pbr-book.org/4ed. Use as a deep reference for radiometry (ch. 4) and reflection models (ch. 9). Heavy maths; not needed to ship a real-time renderer.
- Not free: **Real-Time Rendering 4th ed.** (Akenine-Moller et al.) ch. 9 "Physically Based Shading" is the best textbook treatment; worth buying if the developer wants one book. https://realtimerendering.com/ also has a useful resource page.

## Prerequisites
- Basic vector maths (dot products, normalisation), HLSL basics, a working lit-triangle/cube pipeline (03 Lighting assumes this).

## Feasibility & pitfalls
- Difficulty: S. Direct-light Cook-Torrance is ~50 lines of HLSL.
- Pitfalls: forgetting alpha = roughness^2; sRGB textures sampled as UNORM (washed-out) or normal maps as SRGB (wrong normals); doing lighting in gamma space; NaNs from n.v = 0 (clamp to small epsilon); roughness 0 creating infinitely small highlights (clamp min roughness ~0.045 like Filament).
- Skip initially: multiscatter compensation, clear coat, anisotropy, sheen, subsurface. All are additive extensions.

## DX12 requirements & fallbacks
- Works at SM 5.1/6.0, FL 11_0+. Nothing special required.
- sRGB: `DXGI_FORMAT_*_SRGB` for colour textures. Flip-model swap chains can't be created with an SRGB format: create the swap chain as `R8G8B8A8_UNORM` and use an `R8G8B8A8_UNORM_SRGB` RTV on the back buffer, or apply the sRGB curve in the tonemap shader.
