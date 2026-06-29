using Godot;
using System;

[Tool]
public partial class AlbedoPass : Node
{
	// Assign these in the inspector.
	[Export] public SubViewport WorldViewport;   // the lit render (holds your 3D scene)
	[Export] public SubViewport AlbedoViewport;  // unshaded copy of the same world
	[Export] public Camera3D SourceCamera;       // your real camera (inside WorldViewport)
	[Export] public Camera3D AlbedoCamera;        // camera inside AlbedoViewport

	public override void _Ready()
	{
		// Render the SAME world in the albedo viewport, but with no lighting.
		AlbedoViewport.World3D = WorldViewport.World3D;
		AlbedoViewport.DebugDraw = Viewport.DebugDrawEnum.Unshaded;
	}

	public override void _Process(double delta)
	{
		// Keep both render targets matched to the window size.
		Vector2I size = GetWindow().Size;
		WorldViewport.Size = size;
		AlbedoViewport.Size = size;

		// Lock the albedo camera onto the real one so the two renders line up.
		AlbedoCamera.GlobalTransform = SourceCamera.GlobalTransform;
		AlbedoCamera.Fov = SourceCamera.Fov;
		AlbedoCamera.Near = SourceCamera.Near;
		AlbedoCamera.Far = SourceCamera.Far;
	}
}
