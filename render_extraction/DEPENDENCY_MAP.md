# Renderer Dependency Map (pass 1: `#include` scan)

Generated from `#include` lines on branch `render-extraction` (base 860c27364f).
Pass 2 will verify against `compile_commands.json`.

"Renderer" = `servers/rendering/` + `drivers/{vulkan,gles3,d3d12,metal}`.

## Size

| Component | Files | Lines (incl. GLSL) |
|---|---:|---:|
| servers/rendering | 270 | 290,511 |
| drivers/gles3 | 51 | 46,635 |
| drivers/metal | 15 | 10,616 |
| drivers/d3d12 | 12 | 9,420 |
| drivers/vulkan | 9 | 8,417 |

`RenderingServer` exposes **548 pure-virtual methods**.

## Headline finding

The public seam already exists and is mostly respected: **2,498 call sites** outside the
renderer go through `RenderingServer::get_singleton()` / `RS::get_singleton()`
(scene/resources 567, scene/3d 479, editor/scene 255, scene/main 247, scene/2d 226, ...).
Those don't need to change for extraction; they only need `rendering_server.h` to stay
self-contained.

The real work is the **~80 places where code bypasses the singleton** and includes renderer
internals, plus **~20 places where the renderer reaches up into higher layers**.

## A. Renderer -> outside (upward/sideways layering violations)

Allowed and expected: `core/*` (templates 99, config 35, math 29, object 20, io 19,
variant 12, os 12, string 10). Extraction keeps a dependency on `core` (see Strategy).

Violations:

| From | Includes | Notes |
|---|---|---|
| renderer_viewport.cpp:44 | main/main.h | Only for `Main::get_boot_splash_bg_color()` (line 841). Easy: pass color in. |
| renderer_scene_cull.cpp:48 | scene/main/node.h | No `Node` usage found. Likely dead include. |
| dummy/rasterizer_dummy.h:43 | scene/resources/mesh.h | No `Mesh` usage found. Likely dead include. |
| gles3/rasterizer_scene_gles3.h:50 | scene/resources/mesh.h | No `Mesh` usage found. Likely dead include. |
| shader_language.h:49, shader_preprocessor.h:48 | scene/resources/shader_include.h | Real use: `Ref<ShaderInclude>` in preprocessor API. Needs an include-resolver callback. |
| rendering_server.h:47, rendering_device.h:47, rendering_context_driver.h:41, d3d12 context | servers/display/display_server.h | Window handles / vsync types. Need a small window-surface interface. |
| rendering_method.h:49, renderer_viewport.cpp:51, renderer_compositor.cpp:44, renderer_rd/effects/vrs.cpp:45 | servers/xr/* | XR is wired directly into the renderer. Needs an XR-frame interface. |
| gles3/rasterizer_scene_gles3.cpp:52-53 | servers/camera/* | Camera feed textures. |
| rendering_device.cpp:47-50, rendering_device_binds.cpp:41-43 | modules/glslang/shader_compile.h, modules_enabled.gen.h | Core renderer depends on an optional module; module in turn includes renderer (cycle). |
| vulkan/rendering_device_driver_vulkan.cpp:48-50 | platform/android/* | Platform code inside a driver. |
| gles3/* (10 files) | platform_gl.h | Per-platform GL header; acceptable as a build-provided header. |

## B. Outside -> renderer internals (bypassing `RenderingServer`)

Grouped by why they reach in:

1. **Bootstrapping the renderer** (platform display servers create the compositor/device):
   platform/{linuxbsd x11+wayland, windows, macos, android, web}, drivers/apple_embedded,
   servers/display/display_server_headless.h, main/main.cpp, tests.
   Includes `renderer_compositor.h`, `renderer_rd/renderer_compositor_rd.h`,
   `rendering_device.h`, `dummy/rasterizer_dummy.h`, `rendering_server_default.h`.
   -> This becomes the **renderer factory/registration API** (the swap point).
2. **Shader language tooling** (editor + scene/resources/shader*, visual_shader,
   register_server_types): `shader_language.h`, `shader_preprocessor.h`, `shader_types.h`,
   `shader_warnings.h`.
   -> Shader *language* is arguably not the renderer; candidate to move to its own
   library that renderers consume.
3. **RenderingDevice users** (lightmapper_rd, betsy, glslang, editor shader file importer,
   core/config/engine.cpp, os_linuxbsd/os_windows): `rendering_device*.h`.
   -> `RenderingDevice` is a public low-level API; needs to be either part of the
   interface or optional per renderer.
4. **Deep reach into RD internals** (worst offenders):
   - modules/openxr/extensions/platform/openxr_vulkan_extension.cpp -> `renderer_rd/effects/copy_effects.h`, `renderer_rd/storage_rd/texture_storage.h`
   - servers/xr/xr_interface_extension.cpp -> `renderer_rd/storage_rd/texture_storage.h`
   - servers/xr/xr_vrs.cpp -> `renderer_scene_render.h`
   - editor/export/shader_baker_export_plugin.* -> `renderer_scene_render_rd.h`, `storage_rd/material_storage.h`, `shader_rd.h`
   - modules/raycast/raycast_occlusion_cull.h -> `renderer_scene_occlusion_cull.h`
   - scene/resources/compositor.h -> `storage/render_data.h`
   - `rendering_server_globals.h` (RSG, the renderer's internal globals) used by
     scene/main/viewport.cpp, editor_resource_preview.cpp, lightmapper_rd, openxr (5 files), webxr.
5. **XR interfaces** (mobile_vr, openxr, webxr) -> `renderer_compositor.h`.

## Strategy implications

- Keep `core/` as a dependency of the renderer library for now (it's the foundation
  everything uses: containers, math, RID, Variant). Splitting `core` is a separate project.
- Order of work, cheapest first:
  1. Remove dead upward includes (A: node.h, mesh.h x2) and the `Main::` call.
  2. Introduce a **renderer registration/factory** so platforms stop including
     `renderer_compositor_rd.h` / `rasterizer_dummy.h` directly (B1). This is the swap point.
  3. Replace `RSG::` usage outside the renderer with `RenderingServer` API (B4).
  4. XR / camera / display interfaces (A).
  5. Break glslang <-> renderer cycle.
  6. Decide where shader language tooling lives (B2).

## Bug / cleanup candidates (to verify after build)

- ~~Dead includes: renderer_scene_cull.cpp:48, rasterizer_dummy.h:43, rasterizer_scene_gles3.h:50.~~ Removed (EVIDENCE #1-3).
- `core/error/error_macros.cpp:50` includes `scene/main/node.h` (core -> scene). Not renderer, but blocks a clean `core` later.
