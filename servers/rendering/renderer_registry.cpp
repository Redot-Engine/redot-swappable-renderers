/**************************************************************************/
/*  renderer_registry.cpp                                                 */
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

/**
 * @file renderer_registry.cpp
 *
 * Implementation of RendererRegistry. See renderer_registry.h.
 */

#include "renderer_registry.h"

#include "core/error/error_macros.h"
#include "core/os/os.h"

LocalVector<RendererRegistry::RendererInfo> RendererRegistry::renderers;
const RendererRegistry::RendererInfo *RendererRegistry::current = nullptr;
LocalVector<void (*)()> RendererRegistry::class_registrations;

void RendererRegistry::register_renderer(const RendererInfo &p_info) {
	ERR_FAIL_COND_MSG(p_info.name.is_empty(), "Renderer name can't be empty.");
	ERR_FAIL_NULL_MSG(p_info.make_current, vformat("Renderer '%s' has no make_current function.", p_info.name));
	ERR_FAIL_COND_MSG(find(p_info.name) != nullptr, vformat("Renderer '%s' is already registered.", p_info.name));
	renderers.push_back(p_info);
}

void RendererRegistry::add_class_registration(void (*p_func)()) {
	ERR_FAIL_NULL(p_func);
	class_registrations.push_back(p_func);
}

void RendererRegistry::register_classes() {
	for (void (*func)() : class_registrations) {
		func();
	}
}

void RendererRegistry::clear() {
	renderers.clear();
	class_registrations.clear();
	current = nullptr;
}

const RendererRegistry::RendererInfo *RendererRegistry::find(const String &p_name) {
	for (const RendererInfo &info : renderers) {
		if (info.name == p_name) {
			return &info;
		}
	}
	return nullptr;
}

Vector<String> RendererRegistry::get_renderer_names() {
	Vector<String> names;
	for (const RendererInfo &info : renderers) {
		names.push_back(info.name);
	}
	return names;
}

Vector<String> RendererRegistry::get_renderers_for_driver(const String &p_driver) {
	Vector<String> names;
	for (const RendererInfo &info : renderers) {
		if (info.drivers.has(p_driver)) {
			names.push_back(info.name);
		}
	}
	return names;
}

bool RendererRegistry::has_feature(Feature p_feature) {
	// Looked up by name rather than `current`, so this also works before a renderer is made current
	// (e.g. while scenes are loaded by tools that never create a renderer).
	const RendererInfo *info = find(OS::get_singleton()->get_current_rendering_method());
	return info && (info->features & p_feature);
}

RendererRegistry::ContextMode RendererRegistry::get_current_context_mode() {
	const RendererInfo *info = find(OS::get_singleton()->get_current_rendering_method());
	return info ? info->context_mode : CONTEXT_NONE;
}

Error RendererRegistry::make_current(const ContextInfo &p_context) {
	if (p_context.driver == "dummy") {
		return make_current("dummy", p_context);
	}
	return make_current(OS::get_singleton()->get_current_rendering_method(), p_context);
}

Error RendererRegistry::make_current(const String &p_name, const ContextInfo &p_context) {
	const RendererInfo *info = find(p_name);
	ERR_FAIL_NULL_V_MSG(info, ERR_UNAVAILABLE, vformat("Renderer '%s' is not registered.", p_name));
	ERR_FAIL_COND_V_MSG(!info->drivers.has(p_context.driver), ERR_UNAVAILABLE, vformat("Renderer '%s' does not support rendering driver '%s'.", p_name, p_context.driver));
	info->make_current(p_context);
	current = info;
	return OK;
}
