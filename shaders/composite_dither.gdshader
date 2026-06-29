shader_type canvas_item;

// Additive light-buffer dither.
// base  = albedo tinted by ambient/sky      (the flat "unlit" draw)
// light = lit - base                        (dynamic lights, black where none)
// Only `light` gets dithered, then it's added back over base.
// lit_tex    -> ViewportTexture of the lit WorldViewport
// albedo_tex -> ViewportTexture of the unshaded AlbedoViewport

uniform sampler2D lit_tex : filter_nearest;
uniform sampler2D albedo_tex : filter_nearest;
uniform int pixel_size = 4;          // pixel-art block size
uniform float shade_steps = 4.0;     // bands in the dithered light
uniform float dither_strength = 0.8; // 0 = hard bands, 1 = full grain
uniform vec3 ambient_color : source_color = vec3(0.51, 0.83, 1.0); // match Environment ambient
uniform float ambient_energy = 1.0;  // raise to fold more flat light into the clean base

const float bayer[16] = float[](
     0.0,  8.0,  2.0, 10.0,   12.0,  4.0, 14.0,  6.0,
     3.0, 11.0,  1.0,  9.0,   15.0,  7.0, 13.0,  5.0
);

void fragment() {
	vec2 vp = vec2(textureSize(lit_tex, 0));
	float ps = float(pixel_size);

	// Pixelate everything: sample the centre of each block.
	vec2 block = floor(FRAGCOORD.xy / ps);
	vec2 uv = (block * ps + ps * 0.5) / vp;

	vec3 alb = texture(albedo_tex, uv).rgb;
	vec3 lit = texture(lit_tex, uv).rgb;

	// Flat ambient base (clean, never dithered).
	vec3 base = alb * ambient_color * ambient_energy;

	// Dynamic light only — black where nothing beyond ambient lands.
	vec3 light = max(lit - base, vec3(0.0));

	// Dither + quantize JUST the light buffer.
	vec2 dcoord = block; // swap for world-anchoring later
	int bx = int(dcoord.x) % 4;
	int by = int(dcoord.y) % 4;
	float d = (bayer[by * 4 + bx] / 16.0 - 0.5) * dither_strength;
	float steps = shade_steps - 1.0;
	vec3 light_q = max(round(light * steps + d) / steps, vec3(0.0));

	COLOR = vec4(base + light_q, 1.0);
}
