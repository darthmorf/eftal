// Shared dithered-lighting core.
// Dithers/bands only the dynamic (spot/omni) lights; directional + ambient stay smooth.
//
// CONFIGURED GLOBALLY in Project Settings -> Shader Globals (all the dither_* values
// below live there, shared by every material). Toggle the whole effect with `dither_enabled`.
//
// To make ANY material use it:
//   shader_type spatial;
//   #include "res://shaders/dither_lighting.gdshaderinc"
//   void fragment() { ALBEDO = ...your base colour...; }
// The light() function below is supplied by the include - don't redeclare it.

global uniform bool dither_enabled;          // master on/off switch
global uniform float dither_light_steps;     // bands in the dithered light
global uniform float dither_strength;        // 0 = hard bands, 1 = full grain
global uniform float dither_block_size;      // keep equal to the pixelation pixel_size
global uniform float dither_light_gain;      // lifts dim light to full before banding
global uniform bool dither_skip_directional; // true = directional/key light stays smooth

const float _dither_bayer[16] = float[](
     0.0,  8.0,  2.0, 10.0,   12.0,  4.0, 14.0,  6.0,
     3.0, 11.0,  1.0,  9.0,   15.0,  7.0, 13.0,  5.0
);

float _dither_quantize(float energy, vec2 frag) {
	int bx = int(floor(frag.x / dither_block_size)) % 4;
	int by = int(floor(frag.y / dither_block_size)) % 4;
	float d = (_dither_bayer[by * 4 + bx] / 16.0 - 0.5) * dither_strength;
	float steps = max(dither_light_steps - 1.0, 1.0);
	return max(round(energy * steps + d) / steps, 0.0);
}

void light() {
	float energy = clamp(dot(NORMAL, LIGHT), 0.0, 1.0) * ATTENUATION;
	// When enabled, dither every light except the directional key light.
	if (dither_enabled && !(dither_skip_directional && LIGHT_IS_DIRECTIONAL)) {
		energy = clamp(energy * dither_light_gain, 0.0, 1.0); // brighten before banding
		energy = _dither_quantize(energy, FRAGCOORD.xy);
	}
	DIFFUSE_LIGHT += energy * LIGHT_COLOR * ALBEDO;
}
