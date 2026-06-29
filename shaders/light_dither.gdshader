shader_type spatial;

// Per-light dithered / quantized lighting.
// Only the LIGHT term is banded + dithered; ALBEDO (base colour) stays clean.
// Apply this as a ShaderMaterial on the meshes you want affected (ground, character),
// then keep the camera post-process as PIXELATION ONLY so the look isn't double-applied.

uniform sampler2D albedo_tex : source_color, filter_nearest;
uniform vec4 tint : source_color = vec4(1.0);
uniform bool use_texture = true;
uniform float light_levels = 4.0;   // shading bands per light (lower = chunkier)
uniform float block_size = 6.0;     // match the post-process pixel_size so the
                                    // dither lines up with the pixel-art blocks

const float bayer[16] = float[](
     0.0,  8.0,  2.0, 10.0,
    12.0,  4.0, 14.0,  6.0,
     3.0, 11.0,  1.0,  9.0,
    15.0,  7.0, 13.0,  5.0
);

void fragment() {
    vec3 base = tint.rgb * COLOR.rgb;        // COLOR = vertex colour (white if none)
    if (use_texture) base *= texture(albedo_tex, UV).rgb;
    ALBEDO = base;
}

void light() {
    // Direct energy for THIS light, incl. spotlight cone + distance falloff.
    float energy = clamp(dot(NORMAL, LIGHT), 0.0, 1.0) * ATTENUATION;

    // Ordered (Bayer) dither, indexed by pixel-art block so it matches the pixelation.
    int bx = int(floor(FRAGCOORD.x / block_size)) % 4;
    int by = int(floor(FRAGCOORD.y / block_size)) % 4;
    float t = bayer[by * 4 + bx] / 16.0 - 0.5;

    // Dither then quantize the light into bands.
    energy = floor((energy + t / light_levels) * light_levels) / light_levels;
    energy = max(energy, 0.0);

    DIFFUSE_LIGHT += energy * LIGHT_COLOR * ALBEDO;
}
