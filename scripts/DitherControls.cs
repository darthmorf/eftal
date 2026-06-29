using Godot;

// Live editor controls for the global dither shader parameters.
// Tweak these in the inspector (tool script) and the scene updates instantly -
// no need to open Project Settings -> Shader Globals each time.

[Tool]
public partial class DitherControls : Node
{
	private bool _enabled = true;
	private float _lightSteps = 4f;
	private float _strength = 1f;
	private float _blockSize = 4f;
	private float _lightGain = 8f;
	private bool _skipDirectional = true;

	[Export] public bool Enabled { get => _enabled; set { _enabled = value; Push("dither_enabled", value); } }
	[Export(PropertyHint.Range, "2,16,1")] public float LightSteps { get => _lightSteps; set { _lightSteps = value; Push("dither_light_steps", value); } }
	[Export(PropertyHint.Range, "0,1,0.01")] public float Strength { get => _strength; set { _strength = value; Push("dither_strength", value); } }
	[Export(PropertyHint.Range, "1,16,1")] public float BlockSize { get => _blockSize; set { _blockSize = value; Push("dither_block_size", value); } }
	[Export(PropertyHint.Range, "1,40,0.1")] public float LightGain { get => _lightGain; set { _lightGain = value; Push("dither_light_gain", value); } }
	[Export] public bool SkipDirectional { get => _skipDirectional; set { _skipDirectional = value; Push("dither_skip_directional", value); } }

	public override void _Ready()
	{
		Push("dither_enabled", _enabled);
		Push("dither_light_steps", _lightSteps);
		Push("dither_strength", _strength);
		Push("dither_block_size", _blockSize);
		Push("dither_light_gain", _lightGain);
		Push("dither_skip_directional", _skipDirectional);
	}

	private static void Push(string name, Variant value)
		=> RenderingServer.GlobalShaderParameterSet(name, value);
}
