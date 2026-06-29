shader_type spatial;
render_mode unshaded, fog_disabled, cull_disabled, depth_test_disabled;

uniform sampler2D screen_tex : hint_screen_texture, filter_nearest;

uniform int pixel_size = 2;          // pixel-art block size (applies to EVERYTHING)
uniform float shade_steps = 8.0;     // brightness bands (the dithered part)
uniform float dither_strength = 0.5; // 0 = hard bands, 1 = full grain

const float bayer[16] = float[](
     0.0,  8.0,  2.0, 10.0,
    12.0,  4.0, 14.0,  6.0,
     3.0, 11.0,  1.0,  9.0,
    15.0,  7.0, 13.0,  5.0
);

void vertex() {
    POSITION = vec4(VERTEX.xy, 1.0, 1.0); // needs a QuadMesh sized (2,2)
}

void fragment() {
    // Pixelate everything: sample the centre of each block.
    vec2 block = floor(FRAGCOORD.xy / float(pixel_size));
    vec2 uv = (block * float(pixel_size) + float(pixel_size) * 0.5) / VIEWPORT_SIZE;
    vec3 col = texture(screen_tex, uv).rgb;

    // Dither + quantize ONLY brightness, keep hue.
    // Flat-lit surfaces = constant brightness -> stay clean.
    // Lighting gradients = changing brightness -> get the dithered banding.
    int bx = int(block.x) % 4;
    int by = int(block.y) % 4;
    float d = (bayer[by * 4 + bx] / 16.0 - 0.5) * dither_strength;

    float steps = shade_steps - 1.0;
    float lum = dot(col, vec3(0.299, 0.587, 0.114));
    float q = round(lum * steps + d) / steps;
    col *= q / max(lum, 1e-4); // rescale colour to the stepped brightness

    ALBEDO = col;
}
