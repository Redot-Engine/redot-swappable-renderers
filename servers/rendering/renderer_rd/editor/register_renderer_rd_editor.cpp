/**************************************************************************/
/*  register_renderer_rd_editor.cpp                                       */
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
 * @file register_renderer_rd_editor.cpp
 *
 * Implementation of register_renderer_rd_editor.h.
 */

#include "register_renderer_rd_editor.h"

#include "editor/export/editor_export.h"
#include "servers/rendering/renderer_rd/editor/shader_baker_export_plugin.h"
#ifdef VULKAN_ENABLED
#include "servers/rendering/renderer_rd/editor/shader_baker_export_plugin_platform_vulkan.h"
#endif
#ifdef D3D12_ENABLED
#include "servers/rendering/renderer_rd/editor/shader_baker_export_plugin_platform_d3d12.h"
#endif
#ifdef METAL_ENABLED
#include "servers/rendering/renderer_rd/editor/shader_baker_export_plugin_platform_metal.h"
#endif

void register_renderer_rd_editor_plugins() {
	Ref<ShaderBakerExportPlugin> shader_baker_export_plugin;
	shader_baker_export_plugin.instantiate();

#ifdef VULKAN_ENABLED
	Ref<ShaderBakerExportPluginPlatformVulkan> shader_baker_export_plugin_platform_vulkan;
	shader_baker_export_plugin_platform_vulkan.instantiate();
	shader_baker_export_plugin->add_platform(shader_baker_export_plugin_platform_vulkan);
#endif

#ifdef D3D12_ENABLED
	Ref<ShaderBakerExportPluginPlatformD3D12> shader_baker_export_plugin_platform_d3d12;
	shader_baker_export_plugin_platform_d3d12.instantiate();
	shader_baker_export_plugin->add_platform(shader_baker_export_plugin_platform_d3d12);
#endif

#ifdef METAL_ENABLED
	Ref<ShaderBakerExportPluginPlatformMetal> shader_baker_export_plugin_platform_metal;
	shader_baker_export_plugin_platform_metal.instantiate();
	shader_baker_export_plugin->add_platform(shader_baker_export_plugin_platform_metal);
#endif

	EditorExport::get_singleton()->add_export_plugin(shader_baker_export_plugin);
}
