/**************************************************************************/
/*  rasterizer_raw_vulkan_test.h                                          */
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
 * @file rasterizer_raw_vulkan_test.h
 *
 * Example of a self-managed renderer (RendererRegistry::CONTEXT_SELF_MANAGED).
 *
 * It owns its whole Vulkan setup, created from the DisplayServer's native
 * window handles, and only clears the screen to an animated color. Everything
 * else (meshes, textures, canvas, scenes) comes from the dummy renderer.
 */

#include "drivers/vulkan/godot_vulkan.h"
#include "servers/rendering/dummy/rasterizer_dummy.h"

class RasterizerRawVulkanTest : public RasterizerDummy {
	// Device (UnironicDevice in a typical Vulkan tutorial).
	VkInstance instance = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	uint32_t queue_family = 0;
	VkQueue queue = VK_NULL_HANDLE;
	VkCommandPool command_pool = VK_NULL_HANDLE;
	VkCommandBuffer command_buffer = VK_NULL_HANDLE;

	// Swap chain (UnironicSwapChain).
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkFormat swapchain_format = VK_FORMAT_UNDEFINED;
	VkExtent2D swapchain_extent = {};
	LocalVector<VkImage> swapchain_images;
	LocalVector<VkSemaphore> render_finished; // One per swap chain image.

	// Frame synchronization.
	VkSemaphore image_available = VK_NULL_HANDLE;
	VkFence frame_fence = VK_NULL_HANDLE;

	bool initialized = false;

	bool _create_instance_and_surface();
	bool _pick_physical_device();
	bool _create_device();
	bool _create_swapchain();
	void _destroy_swapchain();
	void _draw_frame();

public:
	void initialize() override;
	void end_frame(bool p_present) override;
	void finalize() override;

	static RendererCompositor *_create_current() {
		return memnew(RasterizerRawVulkanTest);
	}

	static void make_current() {
		_create_func = _create_current;
		low_end = false;
	}
};
