shader_type spatial;
// Ready-to-use material with dithered dynamic lighting.
// Drop this on the ground / character; set albedo_tex to that surface's texture.
// All the dither behaviour comes from the shared include below.
#include "res://shaders/dither_lighting.gdshaderinc"

uniform sampler2D albedo_tex : source_color, filter_linear_mipmap_anisotropic;
uniform vec4 albedo_tint : source_color = vec4(1.0);
uniform bool use_texture = true;
uniform bool world_uv = false;   // true = tile by world XZ (good for a flat ground)
uniform float uv_scale = 1.0;

varying vec3 world_pos;

void vertex() {
	world_pos = (MODEL_MATRIX * vec4(VERTEX, 1.0)).xyz;
}

void fragment() {
	vec3 base = albedo_tint.rgb * COLOR.rgb;   // COLOR = vertex colour (white if none)
	if (use_texture) {
		vec2 uv = world_uv ? world_pos.xz * uv_scale : UV * uv_scale;
		base *= texture(albedo_tex, uv).rgb;
	}
	ALBEDO = base;
}
