/**************************************************************************/
/*  renderer_registry.h                                                   */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             REDOT ENGINE                               */
/*                        https://redotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2024-present Redot Engine contributors                   */
/*                                          (see REDOT_AUTHORS.md)        */
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

/**
 * @file renderer_registry.h
 *
 * Registry of available renderers (rendering methods).
 *
 * Renderers register themselves before the DisplayServer is created. The
 * DisplayServer sets up the graphics context for the chosen rendering driver
 * and then asks the registry to make the matching renderer current, instead of
 * hard-coding a renderer class.
 */

#include "core/string/ustring.h"
#include "core/templates/local_vector.h"
#include "core/templates/vector.h"

class RendererCompositor;

class RendererRegistry {
public:
	/// How the renderer obtains access to the GPU.
	enum ContextMode {
		/// The DisplayServer creates a RenderingContextDriver and RenderingDevice.
		CONTEXT_RD,
		/// The DisplayServer creates an OpenGL / OpenGL ES context.
		CONTEXT_GL,
		/// No GPU access is needed.
		CONTEXT_NONE,
		/// The renderer creates and owns its graphics API context from native window handles.
		CONTEXT_SELF_MANAGED,
	};

	/// Optional features a renderer may support. Engine code asks for features
	/// instead of comparing rendering method names.
	enum Feature : uint64_t {
		FEATURE_GEOMETRY_TRANSPARENCY = 1ull << 0,
		FEATURE_VISIBILITY_RANGE_FADE = 1ull << 1,
		FEATURE_LIGHT_PROJECTORS = 1ull << 2,
		FEATURE_PARTICLE_TRAILS = 1ull << 3,
		FEATURE_PARTICLE_SUB_EMITTERS = 1ull << 4,
		FEATURE_VOLUMETRIC_FOG = 1ull << 5,
		FEATURE_VOXEL_GI = 1ull << 6,
		FEATURE_DECALS = 1ull << 7,
		FEATURE_DOF_BLUR = 1ull << 8,
		FEATURE_AUTO_EXPOSURE = 1ull << 9,
		FEATURE_SSR = 1ull << 10,
		FEATURE_SSAO = 1ull << 11,
		FEATURE_SSIL = 1ull << 12,
		FEATURE_SDFGI = 1ull << 13,
		/// Glow levels, blend modes, strength and mix (otherwise only basic glow).
		FEATURE_GLOW_FULL = 1ull << 14,
		/// Editor debug draw modes that need the RenderingDevice scene buffers.
		FEATURE_DEBUG_DRAW_MODES = 1ull << 15,
		/// Editor debug draw modes that need the clustered (Forward+) scene buffers.
		FEATURE_DEBUG_DRAW_MODES_CLUSTERED = 1ull << 16,
		/// 3D scaling modes other than bilinear, including MetalFX Spatial.
		FEATURE_SCALING_3D_ADVANCED = 1ull << 17,
		/// FSR 1, FSR 2 and MetalFX Temporal 3D scaling.
		FEATURE_SCALING_3D_FSR = 1ull << 18,
		FEATURE_SCREEN_SPACE_AA = 1ull << 19,
		FEATURE_TAA = 1ull << 20,
		/// `hint_normal_roughness_texture` in shaders.
		FEATURE_NORMAL_ROUGHNESS_BUFFER = 1ull << 21,
		/// Shader built-ins missing from GLSL ES 3.0, such as `fma()` and coarse/fine derivatives.
		FEATURE_SHADER_DESKTOP_BUILTINS = 1ull << 22,
	};

	/// Information about the context the DisplayServer created, passed to the renderer.
	struct ContextInfo {
		String driver;
		/// CONTEXT_GL only: true for desktop OpenGL, false for OpenGL ES / WebGL.
		bool gles_over_gl = false;
	};

	struct RendererInfo {
		/// Rendering method name, as used by `--rendering-method` and project settings.
		String name;
		/// Rendering drivers this renderer can run on.
		Vector<String> drivers;
		ContextMode context_mode = CONTEXT_RD;
		/// Bitmask of Feature values.
		uint64_t features = 0;
		/// Prepares the renderer and sets `RendererCompositor::_create_func`.
		void (*make_current)(const ContextInfo &p_context) = nullptr;
	};

private:
	static LocalVector<RendererInfo> renderers;
	static const RendererInfo *current;
	static LocalVector<void (*)()> class_registrations;

public:
	static void register_renderer(const RendererInfo &p_info);
	/// Adds a function that registers a renderer's scriptable classes with ClassDB.
	/// Called from register_server_types(), after the server base classes exist.
	static void add_class_registration(void (*p_func)());
	static void register_classes();
	static void clear();

	static const RendererInfo *find(const String &p_name);
	static const RendererInfo *get_current() { return current; }
	static Vector<String> get_renderer_names();
	static Vector<String> get_renderers_for_driver(const String &p_driver);

	/// Whether the renderer for the current rendering method supports a feature.
	/// Unknown rendering methods support nothing.
	static bool has_feature(Feature p_feature);
	/// Context mode of the renderer for the current rendering method (CONTEXT_NONE if unknown).
	static ContextMode get_current_context_mode();

	/// Makes the renderer for the current rendering method current.
	/// The "dummy" driver always selects the "dummy" renderer.
	static Error make_current(const ContextInfo &p_context);
	/// Makes a specific renderer current, ignoring the current rendering method.
	static Error make_current(const String &p_name, const ContextInfo &p_context);
};
