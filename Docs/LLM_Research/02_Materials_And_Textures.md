# 02 Materials and Textures

Status: complete. Researched 2026-11-23.

## Summary
- **glTF 2.0 metallic-roughness** is the material model to adopt: `baseColor` (sRGB texture * factor), `metallicRoughness` texture (**G = roughness, B = metallic**, linear), `normal` (tangent space, linear, with `scale`), `occlusion` (R channel, linear), `emissive` (sRGB). Plus `alphaMode` (OPAQUE/MASK/BLEND) and `doubleSided`. Matches the BRDF from 01 directly. Extensions (KHR_materials_*: clearcoat, specular, transmission, emissive_strength...) are optional later.
  - Careful: some secondary docs say R = metallic; the spec says metallic is in **B**, roughness in **G**. Verify against the spec.
- **Normal mapping**: tangent-space normals need a per-vertex tangent frame. glTF `TANGENT` is vec4, `w` = handedness; bitangent = `cross(N, T.xyz) * w`. If a file has no tangents, the spec says generate them with **MikkTSpace** (the same algorithm Blender/Substance bake with; mismatched tangent frames = visible seams). Green-channel convention: glTF/Blender/DirectX differ (OpenGL "Y+" vs DirectX "Y-"); glTF uses OpenGL-style (+Y up) with UV origin top-left.
- **Texture compression (BCn)**: 4x4-block formats, GPU decompresses on sample. Use:
  - BC7 (UNORM_SRGB) for base colour (+alpha), BC1 if size matters more than quality.
  - BC5 for normal maps (2 channels, reconstruct Z in shader).
  - BC4 for single channels (roughness/AO), or BC7 for packed ORM.
  - BC6H for HDR (environment maps).
- **Mipmaps**: required for quality (no shimmering) and performance. D3D12 has no `GenerateMips`; either pre-generate offline (texconv, recommended) or write a compute shader downsampler. Block-compressed textures can't be generated on GPU at load easily, so offline is the practical route.
- **Pipeline recommendation for a solo dev**: offline convert textures to DDS with texconv (BCn + full mip chain), load DDS at runtime with a small custom loader (DDS is a simple header + raw data) or Microsoft's `DDSTextureLoader12`.

## Sources (easiest first)
1. **LearnOpenGL - Normal Mapping** (Advanced Lighting section of https://learnopengl.com). Easy. Best intro to tangent space, TBN matrix, why tangents are needed. GLSL.
2. **Khronos glTF 2.0 specification** (HTML). Medium, reference. https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html. Read: "Materials" section (metallic-roughness, normal/occlusion/emissive, alpha), Appendix B "BRDF Implementation" (gives the exact BRDF glTF assumes, matches 01), "Meshes" (attributes incl. TANGENT). Extensions registry: https://registry.khronos.org/glTF/. Khronos PBR overview page: https://www.khronos.org/gltf/pbr/.
3. **Nathan Reed - Understanding BCn Texture Compression Formats** (blog, 2012, still accurate). Easy-medium. https://www.reedbeta.com/blog/understanding-bcn-texture-compression-formats. Best single explanation of BC1-BC7: how they work and which to use for what.
4. **Microsoft Docs - Texture Block Compression / BC7 format**. Reference. https://learn.microsoft.com/en-us/windows/uwp/graphics-concepts/texture-block-compression, https://learn.microsoft.com/en-us/windows/uwp/graphics-concepts/bc7-format. Official format details; same for D3D11/12.
5. **DirectXTex / texconv wiki** (Microsoft, Chuck Walbourn). Tool docs. https://github.com/Microsoft/DirectXTex/wiki (Texconv page). Command-line converter: e.g. `-f BC7_UNORM_SRGB` for colour, `-f BC5_UNORM` for normals, `-m 0` = full mip chain (default for DDS). Uses GPU (DirectCompute) for BC6H/BC7 if available. Chuck Walbourn's blog: https://walbourn.github.io/directxtex/.
6. **MikkTSpace** (Morten Mikkelsen, reference C implementation, in Blender's source). Medium. Header viewable at https://projects.blender.org/glukoz/blender/src/tag/v3.3.11/intern/mikktspace/mikktspace.h. Plain C (fits no-stdlib); drop-in when a mesh lacks tangents. glTF-Transform's `tangents` function can also bake tangents offline into the asset.

## Prerequisites
- 01 (which channels mean what), working texture upload in DX12 (06).

## Feasibility & pitfalls
- Difficulty: S (material struct + textures), M (MikkTSpace integration, DDS loader).
- Pitfalls: wrong sRGB/UNORM per texture (see 01); normal map green-channel flip; mismatched tangent generation vs baker; BC formats need texture dimensions that are multiples of 4 (texconv `-pow2` or pad); forgetting `w` handedness -> mirrored-UV seams; alpha-tested (MASK) materials need mipmapped alpha handled carefully (alpha shrinks with mips).
- Runtime libraries: DirectXTex itself uses the C++ STL - fine as an **offline tool** (texconv.exe), but don't link it into the engine if keeping no-stdlib; write a minimal DDS reader instead.

## DX12 requirements & fallbacks
- BC1-BC7 are required on all FL 11_0+ hardware. No fallback needed.
- sRGB views: `DXGI_FORMAT_BC7_UNORM_SRGB`, `BC1_UNORM_SRGB`. Normal maps: `BC5_UNORM` (or `BC5_SNORM`).
- Descriptor-table binding (no bindless at SM 6.0 baseline): one table per material with its 5 SRVs is the simple approach (see 06).
