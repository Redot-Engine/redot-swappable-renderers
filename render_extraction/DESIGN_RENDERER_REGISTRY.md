# Design: Renderer Registry (the swap point)

## How it works today

Two independent choices are made at startup:

| Concept | Values | Chosen in |
|---|---|---|
| **Rendering driver** (graphics API / context) | `vulkan`, `d3d12`, `metal`, `opengl3`, `opengl3_es`, `opengl3_angle`, `dummy` | `main/main.cpp` (CLI + project settings), validated against what each DisplayServer reports |
| **Rendering method** (the renderer) | `forward_plus`, `mobile`, `gl_compatibility`, `dummy` | `main/main.cpp`, hard-coded list at main.cpp:2446-2450 and driver mapping at ~2520-2555 |

Then each platform DisplayServer (X11, Wayland, Windows, macOS, macOS embedded,
Android, Web, Apple embedded, headless; 9 places) does:

1. create the graphics context for the driver (`RenderingContextDriverVulkanX11`, `GLManager_X11`, ...) - legitimately platform-specific;
2. **hard-code the renderer**: `RasterizerDummy::make_current()`, `RasterizerGLES3::make_current(bool)`, or `RendererCompositorRD::make_current()`.

`make_current()` just sets the static `RendererCompositor::_create_func`. Inside
`RendererCompositorRD`, the method string picks Forward+ vs Mobile
(renderer_compositor_rd.cpp:381-393).

### Problems for swappable renderers

1. Renderer choice is fused into 9 platform files (step 2 above). Adding a renderer means editing every platform.
2. The method list is hard-coded in `main.cpp` (validation, defaults, method->driver mapping).
3. Method names are string-compared in **~40 files** outside the renderer
   (editor/project_dialog 13, editor_node 11, editor_build_profile 7, renderer_viewport 6,
   scene/3d/* for "not supported in Compatibility" warnings, export plugins, ...).
   A third renderer is unknown to all of them.

## Proposal

A small **renderer registry** in `servers/rendering/`:

```cpp
struct RendererInfo {
    String name;                    // "forward_plus", "my_renderer"
    String display_name;
    Vector<String> drivers;         // drivers it can run on: {"vulkan","d3d12","metal"}
    RendererCompositor *(*create)();
    BitField<RendererFeature> features; // replaces the scattered string checks
};

class RendererRegistry {
    static void register_renderer(const RendererInfo &p_info);
    static const RendererInfo *find(const String &p_name);
    static Vector<String> get_renderers_for_driver(const String &p_driver);
    static bool make_current(const String &p_method, const String &p_driver);
};
```

- Built-in renderers register themselves (Forward+, Mobile, Compatibility, Dummy).
- DisplayServers keep step 1 (context creation) but replace step 2 with
  `RendererRegistry::make_current(method, driver)`.
- `main.cpp` validation/defaults read from the registry instead of hard-coded lists.
- String checks in scene/editor become feature queries
  (e.g. `RS::get_singleton()->has_feature(...)`); done gradually, later phase.

### Where does a renderer get its GPU access? (`ContextMode`)

```cpp
enum ContextMode {
    CONTEXT_RD,           // DisplayServer creates RenderingContextDriver + RenderingDevice (Forward+, Mobile)
    CONTEXT_GL,           // DisplayServer creates the GL/EGL context (Compatibility)
    CONTEXT_NONE,         // no GPU (Dummy)
    CONTEXT_SELF_MANAGED, // renderer owns its graphics API entirely (raw Vulkan, etc.)
};
```

**Decision (user, 2026-10-24): the user's own renderer will be `CONTEXT_SELF_MANAGED`
raw Vulkan.** Reference: `/mnt/disk1/Game_Engine_Design/Unironic_engine/` (Vulkan
tutorial structure: Window -> Device -> SwapChain -> Pipeline; GLFW window).

For self-managed renderers the DisplayServer:
- creates plain windows (no RD/GL context),
- passes a `RendererWindowHost` to the renderer:
  native handles (via existing `window_get_native_handle(DISPLAY_HANDLE/WINDOW_HANDLE)`),
  window size, vsync mode, and window created/resized/destroyed callbacks.

The renderer then does what the tutorial's `UnironicDevice` / `UnironicSwapChain` do
(VkInstance, VkSurfaceKHR from the native handles, device, swapchain), replacing
GLFW's `glfwCreateWindowSurface` with `vkCreateXlibSurfaceKHR` /
`vkCreateWaylandSurfaceKHR` / `vkCreateWin32SurfaceKHR`.

Caveats: features built on `RenderingDevice` (RD-based lightmapper, betsy GPU
compression, `RenderingDevice` scripting API, OpenXR Vulkan path, shader baker)
are unavailable or must be stubbed under a self-managed renderer.

## Registration timing (learned in phase 2)

`main.cpp` validates `--rendering-method` in `Main::setup()`, before the
SERVERS module level. So `register_renderer_types()` runs right before
`initialize_modules(MODULE_INITIALIZATION_LEVEL_CORE)`, and module renderers
register at the CORE level.

## Phases

1. Registry + built-ins register; DisplayServers call the registry. Behaviour identical.
2. `CONTEXT_SELF_MANAGED` + `RendererWindowHost` on Linux (X11, Wayland), proven by a
   tiny raw-Vulkan test renderer that just clears the screen. This is the hook for the
   user's renderer.
3. `main.cpp` lists come from the registry.
4. Feature flags replace method-name string compares in scene/editor.
5. Move the built-in renderers into their own library target behind the registry.
