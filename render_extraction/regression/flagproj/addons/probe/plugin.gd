@tool
extends EditorPlugin

func _enter_tree() -> void:
	print("EPROBE method=", RenderingServer.get_current_rendering_method())
	var env := Environment.new()
	for mode in [Environment.GLOW_BLEND_MODE_ADDITIVE, Environment.GLOW_BLEND_MODE_MIX]:
		env.glow_blend_mode = mode
		for p in env.get_property_list():
			if str(p.name).begins_with("glow"):
				print("EPROBE mode", mode, " ", p.name, " visible=", (p.usage & PROPERTY_USAGE_EDITOR) != 0)
	get_tree().create_timer(2.0).timeout.connect(func(): get_tree().quit())
