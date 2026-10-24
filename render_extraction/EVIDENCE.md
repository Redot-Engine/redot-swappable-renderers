# Evidence Log

Code in or around the renderer that obstructs extraction or is defective.

Each entry records the location, what is wrong, what it costs the extraction,
and its history (`git blame` / commit messages). Intent is only claimed when the
history supports it; otherwise the entry stays factual.

| # | Location | Problem | Cost to extraction | History | Intent evidence |
|---|----------|---------|--------------------|---------|-----------------|
| 1 | servers/rendering/renderer_scene_cull.cpp:48 | Includes `scene/main/node.h` (renderer -> scene) with comment "used only to obtain node paths for physics interpolation warnings", but nothing in the file uses `Node` or the warning macros. The actual Node lookup lives in `core/error/error_macros.cpp:50,234` (itself a core -> scene violation). | Makes the renderer appear to depend on the scene tree. Removed; builds clean. | Added in Godot upstream 2f8ab4a654 (2024-05-26, "Fixed Timestep Interpolation (3D)"), unused from the start in this file. | None. Looks like a leftover from development of that feature. |
| 2 | servers/rendering/dummy/rasterizer_dummy.h:43 | Includes `scene/resources/mesh.h`; no `Mesh` usage. | Every platform display server pulls `scene/` through the dummy renderer. Removed; builds clean. | Carried along by Godot upstream 57e5a33623 (2022-03-12, splitting dummy renderer into files). | None. Refactor leftover. |
| 3 | drivers/gles3/rasterizer_scene_gles3.h:50 | Includes `scene/resources/mesh.h`; no `Mesh` usage. | GL driver appears to depend on scene resources. Removed; builds clean. | Present since Godot upstream 8a10bb7d0d (2021-10-26, GL 3.3 core-profile rewrite). | None. Refactor leftover. |
| 4 | platform/android/display_server_android.cpp:790, drivers/apple_embedded/display_server_apple_embedded.mm:146 | On `RenderingDevice::initialize()` failure, `rendering_device` is set to `nullptr` without `memdelete` (leak). X11/Wayland/Windows free it. | None directly; found while converting platforms to the registry. Fixed in PR 2. | Android: since e167af3737 (2020-03-27, "Display Server support"), pattern kept through 12a519bae2 (2023-12-19 RD split). | None. Copy-paste divergence between platforms. |
| 5 | platform/macos/display_server_macos.mm:3971, platform/macos/display_server_embedded.mm:193 | Return value of `RenderingDevice::initialize()` ignored; continues to `screen_create()` and RD renderer setup on a failed device. | Same as #4. Fixed in PR 2. | macOS DisplayServer since 15a9f94346 (2020-03-07); unchanged by 12a519bae2. | None. |
| 6 | 9 platform DisplayServers | Renderer class hard-coded per platform (`RasterizerDummy` / `RasterizerGLES3` / `RendererCompositorRD::make_current()`). Hidden transitive includes: Wayland got `RenderingServer` only through `rasterizer_dummy.h`. | The core reason renderers weren't swappable. Replaced by `RendererRegistry` on branch `render-extraction`. | Long-standing design; each platform written separately. | None. Design debt, not sabotage. |
| 7 | ~40 files (editor/project_dialog, editor_node, scene/3d/*, export plugins, renderer_viewport, ...) | Rendering method names (`"forward_plus"`, `"mobile"`, `"gl_compatibility"`) string-compared throughout. | Any new renderer is unknown to the editor, warnings and exporters. Planned: feature flags (phase 4). | Accumulated as features were added. | None. |
