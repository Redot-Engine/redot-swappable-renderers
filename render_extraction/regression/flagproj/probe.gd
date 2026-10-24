extends Node3D

func _ready() -> void:
	print("PROBE method=", RenderingServer.get_current_rendering_method())

	var env := Environment.new()
	env.volumetric_fog_enabled = true
	env.ssr_enabled = true
	env.ssao_enabled = true
	env.ssil_enabled = true
	env.sdfgi_enabled = true
	for p in env.get_property_list():
		if str(p.name).begins_with("glow"):
			print("PROBE glow_prop ", p.name, " visible=", (p.usage & PROPERTY_USAGE_EDITOR) != 0)
	env.glow_blend_mode = Environment.GLOW_BLEND_MODE_MIX
	for p in env.get_property_list():
		if p.name == "glow_intensity" or p.name == "glow_mix":
			print("PROBE glow_mix_prop ", p.name, " visible=", (p.usage & PROPERTY_USAGE_EDITOR) != 0)

	var cam := CameraAttributesPractical.new()
	cam.dof_blur_far_enabled = true
	cam.dof_blur_near_enabled = true
	cam.auto_exposure_enabled = true

	var vp := get_viewport()
	for mode in [Viewport.SCALING_3D_MODE_FSR, Viewport.SCALING_3D_MODE_FSR2, Viewport.SCALING_3D_MODE_METALFX_SPATIAL, Viewport.SCALING_3D_MODE_METALFX_TEMPORAL]:
		RenderingServer.viewport_set_scaling_3d_mode(vp.get_viewport_rid(), mode)
		print("PROBE scaling ", mode, " done")
	RenderingServer.viewport_set_screen_space_aa(vp.get_viewport_rid(), RenderingServer.VIEWPORT_SCREEN_SPACE_AA_FXAA)
	RenderingServer.viewport_set_use_taa(vp.get_viewport_rid(), true)

	var sh := Shader.new()
	sh.code = "shader_type spatial;\nuniform sampler2D nr : hint_normal_roughness_texture;\nvoid fragment() { ALBEDO = texture(nr, SCREEN_UV).rgb; }\n"
	print("PROBE shader_params ", sh.get_shader_uniform_list().size())

	var vs := VisualShader.new()
	var ma := VisualShaderNodeMultiplyAdd.new()
	vs.add_node(VisualShader.TYPE_FRAGMENT, ma, Vector2(), 2)
	var der := VisualShaderNodeDerivativeFunc.new()
	der.precision = VisualShaderNodeDerivativeFunc.PRECISION_FINE
	vs.add_node(VisualShader.TYPE_FRAGMENT, der, Vector2(), 3)
	vs.connect_nodes(VisualShader.TYPE_FRAGMENT, 2, 0, 3, 0)
	var der2 := VisualShaderNodeDerivativeFunc.new()
	vs.add_node(VisualShader.TYPE_FRAGMENT, der2, Vector2(), 4)
	vs.connect_nodes(VisualShader.TYPE_FRAGMENT, 3, 0, 4, 0)
	vs.connect_nodes(VisualShader.TYPE_FRAGMENT, 4, 0, 0, 0)
	for line in vs.get_code().split("\n"):
		if "fma" in line or "dFd" in line or "fwidth" in line or "*" in line:
			print("PROBE vscode ", line.strip_edges())
	print("PROBE end")
	get_tree().quit()
