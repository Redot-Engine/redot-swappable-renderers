/**************************************************************************/
/*  rasterizer_raw_vulkan_test.cpp                                        */
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
 * @file rasterizer_raw_vulkan_test.cpp
 *
 * Implementation of RasterizerRawVulkanTest. See rasterizer_raw_vulkan_test.h.
 */

#include "rasterizer_raw_vulkan_test.h"

#include "core/math/color.h"
#include "core/math/math_funcs.h"
#include "core/string/print_string.h"
#include "servers/display/display_server.h"

bool RasterizerRawVulkanTest::_create_instance_and_surface() {
#ifdef USE_VOLK
	if (volkInitialize() != VK_SUCCESS) {
		ERR_PRINT("Could not load the Vulkan loader.");
		return false;
	}
#endif

	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Redot raw Vulkan test renderer";
	app_info.apiVersion = VK_API_VERSION_1_0;

	const char *extensions[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XLIB_SURFACE_EXTENSION_NAME };

	VkInstanceCreateInfo instance_info = {};
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;
	instance_info.enabledExtensionCount = 2;
	instance_info.ppEnabledExtensionNames = extensions;

	VkResult err = vkCreateInstance(&instance_info, nullptr, &instance);
	ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, false, vformat("vkCreateInstance failed: %d", (int)err));
#ifdef USE_VOLK
	volkLoadInstance(instance);
#endif

	// The window belongs to the DisplayServer; build the surface from its native handles
	// (instead of glfwCreateWindowSurface in a GLFW-based engine).
	DisplayServer *ds = DisplayServer::get_singleton();
	VkXlibSurfaceCreateInfoKHR surface_info = {};
	surface_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surface_info.dpy = (Display *)ds->window_get_native_handle(DisplayServer::DISPLAY_HANDLE);
	surface_info.window = (::Window)ds->window_get_native_handle(DisplayServer::WINDOW_HANDLE);

	err = vkCreateXlibSurfaceKHR(instance, &surface_info, nullptr, &surface);
	ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, false, vformat("vkCreateXlibSurfaceKHR failed: %d", (int)err));
	return true;
}

bool RasterizerRawVulkanTest::_pick_physical_device() {
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(instance, &count, nullptr);
	ERR_FAIL_COND_V_MSG(count == 0, false, "No Vulkan devices found.");
	LocalVector<VkPhysicalDevice> devices;
	devices.resize(count);
	vkEnumeratePhysicalDevices(instance, &count, devices.ptr());

	// Prefer a discrete GPU; accept any device with a queue that can draw and present.
	for (int pass = 0; pass < 2; pass++) {
		for (VkPhysicalDevice candidate : devices) {
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(candidate, &props);
			if (pass == 0 && props.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				continue;
			}

			uint32_t family_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(candidate, &family_count, nullptr);
			LocalVector<VkQueueFamilyProperties> families;
			families.resize(family_count);
			vkGetPhysicalDeviceQueueFamilyProperties(candidate, &family_count, families.ptr());

			for (uint32_t i = 0; i < family_count; i++) {
				VkBool32 present = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(candidate, i, surface, &present);
				if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present) {
					physical_device = candidate;
					queue_family = i;
					print_line(vformat("Raw Vulkan test renderer - Using device: %s", props.deviceName));
					return true;
				}
			}
		}
	}
	ERR_FAIL_V_MSG(false, "No Vulkan device can present to this window.");
}

bool RasterizerRawVulkanTest::_create_device() {
	const float priority = 1.0f;
	VkDeviceQueueCreateInfo queue_info = {};
	queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_info.queueFamilyIndex = queue_family;
	queue_info.queueCount = 1;
	queue_info.pQueuePriorities = &priority;

	const char *extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &queue_info;
	device_info.enabledExtensionCount = 1;
	device_info.ppEnabledExtensionNames = extensions;

	VkResult err = vkCreateDevice(physical_device, &device_info, nullptr, &device);
	ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, false, vformat("vkCreateDevice failed: %d", (int)err));
#ifdef USE_VOLK
	volkLoadDevice(device);
#endif
	vkGetDeviceQueue(device, queue_family, 0, &queue);

	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_info.queueFamilyIndex = queue_family;
	err = vkCreateCommandPool(device, &pool_info, nullptr, &command_pool);
	ERR_FAIL_COND_V(err != VK_SUCCESS, false);

	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = 1;
	err = vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);
	ERR_FAIL_COND_V(err != VK_SUCCESS, false);

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	err = vkCreateSemaphore(device, &semaphore_info, nullptr, &image_available);
	ERR_FAIL_COND_V(err != VK_SUCCESS, false);

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	err = vkCreateFence(device, &fence_info, nullptr, &frame_fence);
	ERR_FAIL_COND_V(err != VK_SUCCESS, false);
	return true;
}

bool RasterizerRawVulkanTest::_create_swapchain() {
	VkSurfaceCapabilitiesKHR caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &caps);
	if (caps.currentExtent.width == 0 || caps.currentExtent.height == 0) {
		return false; // Minimized.
	}

	uint32_t format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
	ERR_FAIL_COND_V(format_count == 0, false);
	LocalVector<VkSurfaceFormatKHR> formats;
	formats.resize(format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats.ptr());
	VkSurfaceFormatKHR format = formats[0];
	for (const VkSurfaceFormatKHR &f : formats) {
		if (f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			format = f;
			break;
		}
	}

	swapchain_format = format.format;
	swapchain_extent = caps.currentExtent;
	if (swapchain_extent.width == UINT32_MAX) {
		Size2i size = DisplayServer::get_singleton()->window_get_size();
		swapchain_extent = { (uint32_t)size.width, (uint32_t)size.height };
	}

	uint32_t image_count = caps.minImageCount + 1;
	if (caps.maxImageCount > 0 && image_count > caps.maxImageCount) {
		image_count = caps.maxImageCount;
	}

	VkSwapchainCreateInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.surface = surface;
	info.minImageCount = image_count;
	info.imageFormat = format.format;
	info.imageColorSpace = format.colorSpace;
	info.imageExtent = swapchain_extent;
	info.imageArrayLayers = 1;
	// Cleared with a transfer command, so no render pass is needed.
	info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.preTransform = caps.currentTransform;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.presentMode = VK_PRESENT_MODE_FIFO_KHR; // Always available; vsync on.
	info.clipped = VK_TRUE;

	VkResult err = vkCreateSwapchainKHR(device, &info, nullptr, &swapchain);
	ERR_FAIL_COND_V_MSG(err != VK_SUCCESS, false, vformat("vkCreateSwapchainKHR failed: %d", (int)err));

	vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr);
	swapchain_images.resize(image_count);
	vkGetSwapchainImagesKHR(device, swapchain, &image_count, swapchain_images.ptr());

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	render_finished.resize(image_count);
	for (uint32_t i = 0; i < image_count; i++) {
		vkCreateSemaphore(device, &semaphore_info, nullptr, &render_finished[i]);
	}
	return true;
}

void RasterizerRawVulkanTest::_destroy_swapchain() {
	if (device == VK_NULL_HANDLE) {
		return;
	}
	vkDeviceWaitIdle(device);
	for (VkSemaphore semaphore : render_finished) {
		vkDestroySemaphore(device, semaphore, nullptr);
	}
	render_finished.clear();
	swapchain_images.clear();
	if (swapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(device, swapchain, nullptr);
		swapchain = VK_NULL_HANDLE;
	}
}

void RasterizerRawVulkanTest::initialize() {
	RasterizerDummy::initialize();
	initialized = _create_instance_and_surface() && _pick_physical_device() && _create_device();
	if (initialized) {
		_create_swapchain();
	}
}

void RasterizerRawVulkanTest::end_frame(bool p_present) {
	// Unlike the dummy renderer, don't call DisplayServer::swap_buffers(); this renderer presents itself.
	if (initialized && p_present) {
		_draw_frame();
	}
}

void RasterizerRawVulkanTest::_draw_frame() {
	vkWaitForFences(device, 1, &frame_fence, VK_TRUE, UINT64_MAX);

	// Recreate the swap chain when the window was resized.
	VkSurfaceCapabilitiesKHR caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &caps);
	const bool extent_changed = caps.currentExtent.width != UINT32_MAX && (caps.currentExtent.width != swapchain_extent.width || caps.currentExtent.height != swapchain_extent.height);
	if (swapchain == VK_NULL_HANDLE || extent_changed) {
		_destroy_swapchain();
		if (!_create_swapchain()) {
			return;
		}
	}

	uint32_t image_index = 0;
	VkResult err = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, image_available, VK_NULL_HANDLE, &image_index);
	if (err == VK_ERROR_OUT_OF_DATE_KHR) {
		_destroy_swapchain();
		return;
	}
	ERR_FAIL_COND(err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR);

	vkResetFences(device, 1, &frame_fence);
	vkResetCommandBuffer(command_buffer, 0);

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(command_buffer, &begin_info);

	VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

	VkImageMemoryBarrier to_transfer = {};
	to_transfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	to_transfer.srcAccessMask = 0;
	to_transfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	to_transfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	to_transfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	to_transfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	to_transfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	to_transfer.image = swapchain_images[image_index];
	to_transfer.subresourceRange = range;
	vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &to_transfer);

	// Slowly cycle the hue so it's obvious frames are being presented.
	Color color = Color::from_hsv(Math::fmod(get_total_time() * 0.1, 1.0), 0.6, 0.8);
	VkClearColorValue clear = { { color.r, color.g, color.b, 1.0f } };
	vkCmdClearColorImage(command_buffer, swapchain_images[image_index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear, 1, &range);

	VkImageMemoryBarrier to_present = to_transfer;
	to_present.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	to_present.dstAccessMask = 0;
	to_present.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	to_present.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &to_present);

	vkEndCommandBuffer(command_buffer);

	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &image_available;
	submit.pWaitDstStageMask = &wait_stage;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &command_buffer;
	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &render_finished[image_index];
	err = vkQueueSubmit(queue, 1, &submit, frame_fence);
	ERR_FAIL_COND(err != VK_SUCCESS);

	VkPresentInfoKHR present = {};
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.waitSemaphoreCount = 1;
	present.pWaitSemaphores = &render_finished[image_index];
	present.swapchainCount = 1;
	present.pSwapchains = &swapchain;
	present.pImageIndices = &image_index;
	err = vkQueuePresentKHR(queue, &present);
	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
		_destroy_swapchain();
	}
}

void RasterizerRawVulkanTest::finalize() {
	_destroy_swapchain();
	if (device != VK_NULL_HANDLE) {
		vkDestroyFence(device, frame_fence, nullptr);
		vkDestroySemaphore(device, image_available, nullptr);
		vkDestroyCommandPool(device, command_pool, nullptr);
		vkDestroyDevice(device, nullptr);
		device = VK_NULL_HANDLE;
	}
	if (surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(instance, surface, nullptr);
		surface = VK_NULL_HANDLE;
	}
	if (instance != VK_NULL_HANDLE) {
		vkDestroyInstance(instance, nullptr);
		instance = VK_NULL_HANDLE;
	}
	initialized = false;
	RasterizerDummy::finalize();
}
