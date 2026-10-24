# Writing a Self-Managed Renderer

How to plug your own raw-Vulkan renderer into Redot. The working example is
`modules/raw_vulkan_test/` (~350 lines): it owns its Vulkan setup and clears
the screen to an animated color.

## 1. Make it a module

```
modules/my_renderer/
  config.py          can_build(): platform/vulkan/x11 checks
  SCsub              add Vulkan headers + volk + VK_USE_PLATFORM_* define (copy from raw_vulkan_test)
  register_types.*   register with RendererRegistry
  my_renderer.*      your RendererCompositor
```

Build: `scons platform=linuxbsd target=editor module_my_renderer_enabled=yes`
(drop `is_enabled()` from `config.py` to build it by default).

## 2. Register at the CORE level

Renderers must exist before `main.cpp` validates `--rendering-method`, which
happens before the other module levels run.

```cpp
void initialize_my_renderer_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_CORE) {
		return;
	}
	RendererRegistry::RendererInfo info;
	info.name = "my_renderer";                 // --rendering-method my_renderer
	info.drivers.push_back("vulkan");          // --rendering-driver vulkan
	info.context_mode = RendererRegistry::CONTEXT_SELF_MANAGED;
	info.features = 0; // Add RendererRegistry::FEATURE_* bits as you implement them.
	info.make_current = [](const RendererRegistry::ContextInfo &) { MyRenderer::make_current(); };
	RendererRegistry::register_renderer(info);
}
```

Run: `redot --path <project> --rendering-method my_renderer --rendering-driver vulkan`

## 3. What the engine gives you

With `CONTEXT_SELF_MANAGED`, the DisplayServer creates a **plain window** and no
graphics context. Your renderer is created after the window exists. Get the
handles from the DisplayServer:

```cpp
DisplayServer *ds = DisplayServer::get_singleton();
Display *dpy = (Display *)ds->window_get_native_handle(DisplayServer::DISPLAY_HANDLE);
::Window win = (::Window)ds->window_get_native_handle(DisplayServer::WINDOW_HANDLE);
// vkCreateXlibSurfaceKHR(...) instead of glfwCreateWindowSurface(...)
```

Mapping from the Vulkan tutorial structure (`Unironic_engine`):

| Tutorial | In Redot |
|---|---|
| `UnironicWindow` (GLFW) | Owned by the DisplayServer. Use native handles. Don't create a window. |
| `UnironicDevice` | Yours. Create in `initialize()`. Call `volkInitialize/volkLoadInstance/volkLoadDevice` (Redot uses volk). |
| `UnironicSwapChain` | Yours. Recreate when the surface extent changes (check each frame). |
| `FirstApp::run()` loop | Redot runs the loop. Draw and present in `end_frame(p_present)`. |
| `FirstApp::drawFrame()` | Your `end_frame()`. Don't call `DisplayServer::swap_buffers()`; you present. |

## 4. Declare features

`info.features` tells the engine what you support; scene/editor/server code asks
`RendererRegistry::has_feature(RendererRegistry::FEATURE_SSR)` etc. instead of
checking renderer names. Start with `0` and set bits as you implement things:
the user then gets "not available" warnings for everything else, instead of
silent no-ops. Full list: `servers/rendering/renderer_registry.h`.

## 5. The RendererCompositor contract

Start by subclassing `RasterizerDummy` (as the example does): every subsystem
returns a do-nothing implementation, so the engine runs while you only
implement presentation. Then replace subsystems one at a time:

| Getter | What it is | Order to tackle |
|---|---|---|
| `get_texture_storage()` | Textures, render targets, viewports' output | 1st: needed to see anything the engine draws |
| `get_canvas()` | 2D drawing (UI, sprites) | 2nd: lets the editor UI appear |
| `get_mesh_storage()` | Vertex/index buffers, multimesh, skeletons | 3rd |
| `get_material_storage()` | Shaders, materials, global uniforms | with meshes |
| `get_scene()` | 3D scene render (cameras, lights, environment) | 4th |
| `get_light_storage()`, `get_particles_storage()`, `get_gi()`, `get_fog()`, `get_utilities()` | The rest | as needed |

Frame hooks: `initialize()`, `begin_frame(step)`, `blit_render_targets_to_screen(...)`,
`end_frame(present)`, `finalize()`.

## Known limits (today)

- Linux X11 only. Wayland/Windows need the same `renderer_self_managed` change
  their DisplayServers (small; see the X11 diff).
- Single window. Multiple windows/editor sub-windows need per-window surfaces.
- Features built on `RenderingDevice` are unavailable (RD lightmapper, GPU
  texture compression, `RenderingDevice` scripting API, OpenXR Vulkan path).
- Feature warnings use `info.features`. Their texts still name the built-in
  renderers (e.g. "only available when using the Forward+ renderer"), because
  changing them would invalidate translations.
- `--write-movie` and viewport screenshots read back through texture storage,
  so they show nothing until you implement it.
