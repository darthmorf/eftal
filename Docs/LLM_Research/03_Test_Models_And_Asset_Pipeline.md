# 03 Test Models and the Asset Pipeline

Status: complete. Researched 2026-11-23.

## Summary
- Use glTF 2.0 everywhere: well specified, PBR materials built in (02), every DCC tool exports it, and there are reference viewers to compare against. Always test a new renderer feature against an asset that is known to be correct, so bugs are yours, not the asset's.
- Workflow: download asset -> (optional) open in Blender to fix/inspect -> export glTF -> validate -> compare in Khronos Sample Viewer -> load in engine.
- Format choice: `.gltf` + `.bin` + loose textures is easiest to inspect and to post-process textures (texconv to DDS, see 02). `.glb` is one binary file, nicer to ship. Start with `.gltf` (separate) for development.

## Where to get test assets
| Source | Licence | Good for |
|---|---|---|
| **Khronos glTF Sample Assets** https://github.com/KhronosGroup/glTF-Sample-Assets (browse/search: https://github.khronos.org/glTF-Assets/) | Per-model, mostly CC-BY / CC0 (check each model's README) | First tests. Tags: `#core` (no extensions - start here), `#testing` (feature tests), `#showcase` (complex). Old repo glTF-Sample-Models is archived. |
| **Intel Sponza (2022 PBR remake)** https://www.intel.com/content/www/us/en/developer/topic-technology/graphics-research/samples.html | CC-BY per Intel page (another Intel page says ASWF licence - check the package) | Modern PBR medium-size scene; glTF included (~3.7 GB, 4k textures). Add-ons: curtains, ivy, emissive candles. |
| **Classic Sponza** (in Khronos Sample Assets, also McGuire archive) | CC-BY 3.0 | Light, classic, small. Lighting/shadows/GI tests. |
| **McGuire Computer Graphics Archive** https://casual-effects.com/g3d/data10/ | Per-model | Bistro, Sponza, many classic scenes (mostly OBJ -> convert via Blender). |
| **Amazon Lumberyard Bistro (NVIDIA ORCA)** https://developer.nvidia.com/orca/amazon-lumberyard-bistro | CC-BY | Large scene (FBX) for performance + RT stress tests. Convert via Blender. |
| **Poly Haven** https://polyhaven.com (licence: https://www.polyhaven.com/license) | CC0, no attribution required | HDRIs for IBL (05), PBR texture sets, some models. Best source of environment maps. |
| **pbrt-v4 scenes** https://github.com/mmp/pbrt-v4-scenes | Per-scene | Offline reference scenes (e.g. San Miguel); pbrt format, not glTF - mostly for comparison renders. |

## Which models test what (Khronos set, names from the repo)
- Start: `Box`, `BoxTextured`, `Duck` - loader, transforms, textures.
- Non-power-of-2 textures: `BoxTexturedNonPowerOfTwo`.
- PBR correctness: `MetalRoughSpheres` (grid of metallic x roughness - THE BRDF test), `MetalRoughSpheresNoTextures`, `DamagedHelmet` and `FlightHelmet` (full material sets, popular reference), `SciFiHelmet`.
- Normal maps / tangents: `NormalTangentTest`, `NormalTangentMirrorTest` (catches handedness `w` bugs).
- Texture details: `TextureCoordinateTest`, `TextureSettingsTest`, `OrientationTest` (axes/transforms).
- Alpha: `AlphaBlendModeTest` (OPAQUE/MASK/BLEND).
- Scenes: `Sponza`. Large: Intel Sponza, Bistro.
- More from the repo's `#testing` list (https://github.com/KhronosGroup/glTF-Sample-Assets/blob/main/Models/Models-testing.md): `EnvironmentTest` (metal/dielectric spheres under IBL - great for 05), `TextureEncodingTest` + `TextureLinearInterpolationTest` (sRGB handling), `CompareMetallic` / `CompareRoughness` / `CompareNormal` / `CompareAmbientOcclusion`, `DirectionalLight`, `PointLightIntensityTest`, `VertexColorTest`. There is also a `#pbrtest` listing (Models-pbrtest.md).
- All names above verified against `Models/Models.md` on 2026-11-23 (folder names; the non-power-of-two box displays as "Box Textured not 2^N").

## Blender basics for this purpose
- Import: File > Import supports glTF, FBX, OBJ. FBX/OBJ materials often import wrong (non-PBR) and need fixing.
- Materials: the glTF exporter reads the **Principled BSDF** node. Base Color -> baseColor. Metallic/Roughness: packed image with **roughness in G, metallic in B**, Image Texture node colour space = **Non-Color**. Normal map: Image Texture (Non-Color) -> Normal Map node (must stay **Tangent Space**) -> Principled Normal input. Occlusion: needs a custom node group named `glTF Material Output` (or `glTF Settings` in newer versions) with an `Occlusion` input.
- Before export: apply transforms (Ctrl+A > All Transforms) so scale/rotation are baked; check scale (Blender 1 unit = 1 m, glTF is metres, +Y up - exporter converts Blender's +Z up automatically).
- Export options worth knowing: format (glTF Separate vs glb), `+Y Up` (leave on), Tangents (enable to export MikkTSpace tangents so the engine needn't compute them), Apply Modifiers, Materials export, compression off (Draco needs an extra library).

### Blender sources (easiest first)
1. Blender Guru "donut" beginner series (YouTube, search by name) - general Blender literacy; parts 1-6 are enough.
2. Cal Poly - Exporting assets and scenes from Blender https://users.csc.calpoly.edu/~zwood/teaching/csc572/docs/blender-export-guide.html - Easy. Written for students building their own renderer: apply transforms, modifiers, what glTF doesn't support (delta transforms, curves, metaballs).
3. Godot docs - Model export considerations https://docs.godotengine.org/en/stable/tutorials/assets_pipeline/importing_3d_scenes/model_export_considerations.html - Easy, engine-agnostic advice.
4. Mozilla Hubs - Using the Blender glTF exporter https://doc.vr.u-tokyo.ac.jp/docs/Hubs-Cloud/official-docs/creators-using-the-blender-gltf-exporter - Easy; mirror-modifier and Apply Modifiers caveats (it also applies Armature modifiers).
5. Blender manual - glTF 2.0 add-on https://docs.blender.org/manual/en/4.4/addons/import_export/scene_gltf2.html - Reference for exactly how Principled BSDF maps to glTF (check the page for the installed Blender version).

## Validation and reference viewing
- **Khronos glTF Validator** (in-browser, nothing uploaded): https://github.khronos.org/glTF-Validator. Run on every exported file. CLI and npm versions also exist.
- **Khronos glTF Sample Viewer** (reference PBR renderer, drag-and-drop glTF + `.hdr` environment): https://github.khronos.org/glTF-Sample-Viewer-Release/. Use it as the "ground truth" picture to compare the engine against, with the same HDRI.
- Khronos glTF Asset Auditor (checks commerce/size requirements): https://www.khronos.org/gltf/gltf-asset-auditor/ - not needed for games.

## Loading glTF in C++ (no stdlib)
- **cgltf** (Johannes Kuhlmann, MIT, single-header C99) https://github.com/jkuhlmann/cgltf. Fits the no-stdlib rule. `cgltf_parse_file` -> `cgltf_load_buffers` -> walk `data->meshes/materials/nodes`, `cgltf_free`. Supports glTF + glb, core + common extensions (Draco needs an external lib). Accepts custom alloc/free callbacks in `cgltf_options` (verify in header) so it can use the engine allocator later. Used by Filament, bgfx, raylib.
- **tinygltf**: popular, but header-only C++ using STL + nlohmann/json -> conflicts with no-stdlib. Skip.
- What a solo dev needs from the file initially: meshes (POSITION, NORMAL, TEXCOORD_0, TANGENT, indices), materials (pbrMetallicRoughness + normal/occlusion/emissive + alphaMode), node hierarchy (transforms), images (URIs). Skip skins/animations/morph targets/cameras until needed.
- Good practice: convert glTF to an engine-specific binary format offline later ("baking"), so runtime loading is a memcpy. Not needed at first.

## Prerequisites
- 02 (material model), basic DX12 buffer/texture upload (06).

## Feasibility & pitfalls
- Difficulty: S (cgltf + basic meshes), M (full material + hierarchy + texture conversion pipeline).
- Pitfalls: glTF is right-handed, +Y up, metres; D3D conventions are often left-handed - pick one convention and convert once at load (flip Z or handedness of projection). UV origin is top-left in glTF (matches D3D, no flip needed). Accessors can be interleaved/strided and have `byteOffset`s - use cgltf's accessor helpers (`cgltf_accessor_read_float`/`unpack`) at first. Indices can be u8/u16/u32. Missing tangents/normals must be generated.
