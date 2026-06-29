shader_type spatial;
render_mode unshaded, fog_disabled, cull_disabled, depth_test_disabled;

// Pixelation-only post-process for the screen quad.
// Dithered lighting is handled per-material (dither_lighting.gdshaderinc).

uniform sampler2D screen_tex : hint_screen_texture, filter_nearest;
uniform int pixel_size = 4; // keep equal to dither_block_size on your materials

void vertex() {
	POSITION = vec4(VERTEX.xy, 1.0, 1.0); // needs a QuadMesh sized (2,2)
}

void fragment() {
	vec2 vp = VIEWPORT_SIZE;
	vec2 block = floor(FRAGCOORD.xy / float(pixel_size));
	vec2 uv = (block * float(pixel_size) + float(pixel_size) * 0.5) / vp;
	ALBEDO = texture(screen_tex, uv).rgb;
}
