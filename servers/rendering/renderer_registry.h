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
		/// Prepares the renderer and sets `RendererCompositor::_create_func`.
		void (*make_current)(const ContextInfo &p_context) = nullptr;
	};

private:
	static LocalVector<RendererInfo> renderers;
	static const RendererInfo *current;

public:
	static void register_renderer(const RendererInfo &p_info);
	static void clear();

	static const RendererInfo *find(const String &p_name);
	static const RendererInfo *get_current() { return current; }
	static Vector<String> get_renderer_names();
	static Vector<String> get_renderers_for_driver(const String &p_driver);

	/// Makes the renderer for the current rendering method current.
	/// The "dummy" driver always selects the "dummy" renderer.
	static Error make_current(const ContextInfo &p_context);
	/// Makes a specific renderer current, ignoring the current rendering method.
	static Error make_current(const String &p_name, const ContextInfo &p_context);
};
