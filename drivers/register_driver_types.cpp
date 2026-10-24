/**************************************************************************/
/*  register_driver_types.cpp                                             */
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
 * @file register_driver_types.cpp
 *
 * [Add any documentation that applies to the entire file here!]
 */

#include "register_driver_types.h"

#include "drivers/png/image_frames_loader_png.h"
#include "drivers/png/image_loader_png.h"
#include "drivers/png/resource_saver_png.h"
#include "servers/rendering/dummy/rasterizer_dummy.h"
#include "servers/rendering/renderer_registry.h"

#ifdef GLES3_ENABLED
#include "drivers/gles3/rasterizer_gles3.h"
#endif // GLES3_ENABLED

#ifdef RD_ENABLED
#include "servers/rendering/renderer_rd/renderer_compositor_rd.h"
#endif // RD_ENABLED

// The RD renderer's scriptable classes are compiled and registered even without an RD driver,
// so the class list (and extension API) doesn't depend on which drivers are enabled.
#include "servers/rendering/renderer_rd/framebuffer_cache_rd.h"
#include "servers/rendering/renderer_rd/storage_rd/render_data_rd.h"
#include "servers/rendering/renderer_rd/storage_rd/render_scene_buffers_rd.h"
#include "servers/rendering/renderer_rd/storage_rd/render_scene_data_rd.h"
#include "servers/rendering/renderer_rd/uniform_set_cache_rd.h"

#ifdef TOOLS_ENABLED
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
#endif // TOOLS_ENABLED

static Ref<ImageLoaderPNG> image_loader_png;
static Ref<ImageFramesLoaderPNG> image_frames_loader_png;
static Ref<ResourceSaverPNG> resource_saver_png;

void register_core_driver_types() {
	image_loader_png.instantiate();
	ImageLoader::add_image_format_loader(image_loader_png);

	image_frames_loader_png.instantiate();
	ImageFramesLoader::add_image_frames_format_loader(image_frames_loader_png);

	resource_saver_png.instantiate();
	ResourceSaver::add_resource_format_saver(resource_saver_png);
}

void unregister_core_driver_types() {
	ImageLoader::remove_image_format_loader(image_loader_png);
	image_loader_png.unref();

	ImageFramesLoader::remove_image_frames_format_loader(image_frames_loader_png);
	image_frames_loader_png.unref();

	ResourceSaver::remove_resource_format_saver(resource_saver_png);
	resource_saver_png.unref();
}

void register_driver_types() {
}

void unregister_driver_types() {
}

static void _register_rd_classes() {
	GDREGISTER_CLASS(RenderDataRD);
	GDREGISTER_CLASS(RenderSceneDataRD);
	GDREGISTER_CLASS(RenderSceneBuffersRD);
	GDREGISTER_CLASS(FramebufferCacheRD);
	GDREGISTER_CLASS(UniformSetCacheRD);
}

#ifdef TOOLS_ENABLED
static void _register_rd_editor_plugins() {
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
#endif // TOOLS_ENABLED

static void _make_current_dummy(const RendererRegistry::ContextInfo &p_context) {
	RasterizerDummy::make_current();
}

#ifdef GLES3_ENABLED
static void _make_current_gles3(const RendererRegistry::ContextInfo &p_context) {
	RasterizerGLES3::make_current(p_context.gles_over_gl);
}
#endif // GLES3_ENABLED

#ifdef RD_ENABLED
static void _make_current_rd(const RendererRegistry::ContextInfo &p_context) {
	// RendererCompositorRD picks Forward+ or Mobile from the current rendering method.
	RendererCompositorRD::make_current();
}
#endif // RD_ENABLED

void register_renderer_types() {
	RendererRegistry::add_class_registration(_register_rd_classes);
#ifdef TOOLS_ENABLED
	RendererRegistry::add_editor_plugin_registration(_register_rd_editor_plugins);
#endif

#ifdef RD_ENABLED
	Vector<String> rd_drivers;
#ifdef VULKAN_ENABLED
	rd_drivers.push_back("vulkan");
#endif // VULKAN_ENABLED
#ifdef D3D12_ENABLED
	rd_drivers.push_back("d3d12");
#endif // D3D12_ENABLED
#ifdef METAL_ENABLED
	rd_drivers.push_back("metal");
#endif // METAL_ENABLED

	RendererRegistry::RendererInfo forward_plus;
	forward_plus.name = "forward_plus";
	forward_plus.drivers = rd_drivers;
	forward_plus.context_mode = RendererRegistry::CONTEXT_RD;
	forward_plus.make_current = _make_current_rd;
	forward_plus.features = RendererRegistry::FEATURE_GEOMETRY_TRANSPARENCY | RendererRegistry::FEATURE_VISIBILITY_RANGE_FADE |
			RendererRegistry::FEATURE_LIGHT_PROJECTORS | RendererRegistry::FEATURE_PARTICLE_TRAILS | RendererRegistry::FEATURE_PARTICLE_SUB_EMITTERS |
			RendererRegistry::FEATURE_VOLUMETRIC_FOG | RendererRegistry::FEATURE_VOXEL_GI | RendererRegistry::FEATURE_DECALS |
			RendererRegistry::FEATURE_DOF_BLUR | RendererRegistry::FEATURE_AUTO_EXPOSURE |
			RendererRegistry::FEATURE_SSR | RendererRegistry::FEATURE_SSAO | RendererRegistry::FEATURE_SSIL | RendererRegistry::FEATURE_SDFGI |
			RendererRegistry::FEATURE_GLOW_FULL | RendererRegistry::FEATURE_DEBUG_DRAW_MODES | RendererRegistry::FEATURE_DEBUG_DRAW_MODES_CLUSTERED |
			RendererRegistry::FEATURE_SCALING_3D_ADVANCED | RendererRegistry::FEATURE_SCALING_3D_FSR |
			RendererRegistry::FEATURE_SCREEN_SPACE_AA | RendererRegistry::FEATURE_TAA |
			RendererRegistry::FEATURE_NORMAL_ROUGHNESS_BUFFER | RendererRegistry::FEATURE_SHADER_DESKTOP_BUILTINS;
	RendererRegistry::register_renderer(forward_plus);

	RendererRegistry::RendererInfo mobile = forward_plus;
	mobile.name = "mobile";
	mobile.features = RendererRegistry::FEATURE_LIGHT_PROJECTORS | RendererRegistry::FEATURE_PARTICLE_TRAILS | RendererRegistry::FEATURE_PARTICLE_SUB_EMITTERS |
			RendererRegistry::FEATURE_VOXEL_GI | RendererRegistry::FEATURE_DECALS | RendererRegistry::FEATURE_DOF_BLUR |
			RendererRegistry::FEATURE_GLOW_FULL | RendererRegistry::FEATURE_DEBUG_DRAW_MODES |
			RendererRegistry::FEATURE_SCALING_3D_ADVANCED | RendererRegistry::FEATURE_SCREEN_SPACE_AA |
			RendererRegistry::FEATURE_SHADER_DESKTOP_BUILTINS;
	RendererRegistry::register_renderer(mobile);
#endif // RD_ENABLED

#ifdef GLES3_ENABLED
	RendererRegistry::RendererInfo gl_compatibility;
	gl_compatibility.name = "gl_compatibility";
	gl_compatibility.drivers.push_back("opengl3");
	gl_compatibility.drivers.push_back("opengl3_es");
	gl_compatibility.drivers.push_back("opengl3_angle");
	gl_compatibility.context_mode = RendererRegistry::CONTEXT_GL;
	gl_compatibility.make_current = _make_current_gles3;
	gl_compatibility.features = 0; // GLSL ES 3.0 baseline; see RendererRegistry::Feature.
	RendererRegistry::register_renderer(gl_compatibility);
#endif // GLES3_ENABLED

	RendererRegistry::RendererInfo dummy;
	dummy.name = "dummy";
	dummy.drivers.push_back("dummy");
	dummy.context_mode = RendererRegistry::CONTEXT_NONE;
	dummy.make_current = _make_current_dummy;
	// The dummy renderer draws nothing. These are the features engine code has historically
	// treated it as having, kept so headless runs report the same warnings and errors as before.
	dummy.features = RendererRegistry::FEATURE_DOF_BLUR | RendererRegistry::FEATURE_AUTO_EXPOSURE |
			RendererRegistry::FEATURE_GLOW_FULL | RendererRegistry::FEATURE_DEBUG_DRAW_MODES | RendererRegistry::FEATURE_DEBUG_DRAW_MODES_CLUSTERED |
			RendererRegistry::FEATURE_SCALING_3D_ADVANCED | RendererRegistry::FEATURE_SCREEN_SPACE_AA |
			RendererRegistry::FEATURE_SHADER_DESKTOP_BUILTINS;
	RendererRegistry::register_renderer(dummy);
}

void unregister_renderer_types() {
	RendererRegistry::clear();
}
