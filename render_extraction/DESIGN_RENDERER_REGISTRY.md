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

### Where does a custom renderer get its GPU access?

Option **RD-based** (recommended first): the custom renderer implements
`RendererCompositor` on top of `RenderingDevice`, which already abstracts
Vulkan / D3D12 / Metal. The platform context code needs no change.

Option **raw API**: the renderer receives the native context (VkInstance/surface,
GL context) and manages it itself. More freedom, much more platform work.
Can be added later on top of the registry.

## Phases

1. Registry + built-ins register; DisplayServers call the registry. Behaviour identical.
2. `main.cpp` lists come from the registry.
3. Feature flags replace method-name string compares in scene/editor.
4. Move the built-in renderers into their own library target behind the registry.
