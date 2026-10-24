/**************************************************************************/
/*  register_types.cpp                                                    */
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
 * @file register_types.cpp
 *
 * Registers the raw Vulkan test renderer with RendererRegistry.
 *
 * Run with: --rendering-method raw_vulkan_test --rendering-driver vulkan
 */

#include "register_types.h"

#include "rasterizer_raw_vulkan_test.h"

#include "servers/rendering/renderer_registry.h"

static void _make_current_raw_vulkan_test(const RendererRegistry::ContextInfo &p_context) {
	RasterizerRawVulkanTest::make_current();
}

void initialize_raw_vulkan_test_module(ModuleInitializationLevel p_level) {
	// Renderers register at the CORE level so they exist when the rendering method is validated.
	if (p_level != MODULE_INITIALIZATION_LEVEL_CORE) {
		return;
	}

	RendererRegistry::RendererInfo info;
	info.name = "raw_vulkan_test";
	info.drivers.push_back("vulkan");
	info.context_mode = RendererRegistry::CONTEXT_SELF_MANAGED;
	info.make_current = _make_current_raw_vulkan_test;
	RendererRegistry::register_renderer(info);
}

void uninitialize_raw_vulkan_test_module(ModuleInitializationLevel p_level) {
}
