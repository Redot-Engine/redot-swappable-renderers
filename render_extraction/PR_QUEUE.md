# PR Queue

Fixes found during extraction, batched for hand-testing before submission.
Each PR is its own branch off `master`, containing only the fix (no notes).
Nothing is pushed until the batch is hand-tested and approved. PRs go from the
fork only, never `Redot-Engine/redot-engine`.

All six touch disjoint files and merge cleanly together; branch
`test/pr-batch` (in the `/mnt/disk2/Redot/pr-build` worktree) is master + all
six, for testing the whole batch with one build.

**Batch verification (2026-10-24, `test/pr-batch`):** Linux and Windows (MinGW) build
with no errors or warnings; renders pixel-identical to unmodified 26.3 (Forward+,
Mobile, Compatibility on opengl3 and opengl3_es); feature probe identical
(forward_plus, mobile, gl_compatibility, dummy, headless); extension API dump
identical; editor starts cleanly with Vulkan and Compatibility; `--test-rd-creation`
exits 0 like before; .pck export bakes the same 345 shader steps.
Not covered here: macOS/iOS/Android (PR 2), Windows runtime incl. ANGLE (PR 4),
XR hardware (PR 6).

| # | Branch | Summary | How to test by hand | Status |
|---|--------|---------|---------------------|--------|
| 1 | `fix/unused-renderer-scene-includes` | Remove unused `scene/` includes from renderer (EVIDENCE #1-3) | Build editor, open any project, confirm 3D and 2D scenes render. | Ready for testing |
| 2 | `fix/rd-init-failure-cleanup` | Free `RenderingDevice` on init failure (Android, Apple embedded); check init result on macOS + macOS embedded (EVIDENCE #4-5) | Needs Android / macOS / iOS hardware. Normal startup on each with Vulkan/Metal must be unchanged. Failure path is hard to trigger by hand; review-only is reasonable. | Ready for testing (not compile-tested here: no cross toolchains) |
| 3 | `fix/remove-dead-gpu-probe-renderer-calls` | Remove dead `RasterizerGLES3::make_current()` from the GPU probe subprocess (EVIDENCE #8) | Linux/Windows: start the editor normally with both a Vulkan and a Compatibility project; the RD/GL probe runs at startup. Also `redot --test-rd-creation` should exit without crashing. | Ready for testing |
| 4 | `fix/egl-owns-angle-flipped-y` | EGL owns the ANGLE flipped-Y flag instead of pushing it into the GL renderer (EVIDENCE #9) | Windows only: run a Compatibility project with `--rendering-driver opengl3_angle`; the image must not be upside down (check text and the editor 2D view). Other platforms: unaffected. | Ready for testing |
| 5 | `fix/spirv-reflect-build-location` | Build spirv-reflect from `servers/rendering/` (EVIDENCE #12) | Build succeeds; Forward+ project runs (RenderingDevice uses spirv-reflect for every shader). | Ready for testing |
| 6 | `fix/xr-render-target-texture-abstract` | XR render-target texture via `RendererTextureStorage`; OpenXR GL uses abstract storage; remove dead RD includes in OpenXR Vulkan (EVIDENCE #11) | Ideally an OpenXR headset (or the OpenXR simulator) with a Forward+ and a Compatibility project. Without XR hardware: build succeeds, editor starts. | Ready for testing |

---

## Draft PR descriptions

### 3. Remove dead RasterizerGLES3::make_current() calls from GPU probes

The RenderingDevice + OpenGL creation probe (`--test-rd-creation`) runs in a subprocess that exits right after the probe. `RasterizerGLES3::make_current()` only sets static state (the renderer create function and `gles_over_gl`), which nothing reads before the process exits, and the RenderingDevice test that follows doesn't use it.

Removing it also drops the OS layer's dependency on the GLES3 renderer header on Linux and Windows.

Tested: Linux editor build and Windows (MinGW) cross-build; editor startup with Vulkan and Compatibility projects.

### 4. Let EGLManager own the ANGLE surface flipped-Y state

`EGLManager::window_make_current()` pushed the current window's ANGLE surface orientation into the GLES3 renderer via `RasterizerGLES3::set_screen_flipped_y()`, so the GL context layer depended on the renderer.

The flag describes the current EGL surface, so `EGLManager` now stores it and `RasterizerGLES3` reads it through `EGLManager::is_current_window_flipped_y()`. Behavior is unchanged: the renderer sees the orientation of the window most recently made current, as before.

Tested: Windows (MinGW) cross-build. Needs a Windows run with `opengl3_angle`.

### 5. Build spirv-reflect from servers/rendering instead of renderer_rd

The SPIR-V reflection library is used by RenderingDevice (`rendering_device_commons.cpp`), not by the RD renderer, but its build hookup lived in `servers/rendering/renderer_rd/`. Move it next to the code that uses it. No change to what gets built.

Tested: Linux and Windows builds; Forward+ / Mobile render identically.

### 6. XR: access render target textures through RendererTextureStorage

- `XRInterfaceExtension::get_render_target_texture()` used the RD `TextureStorage` singleton directly. It now calls the new `RendererTextureStorage::render_target_get_rd_texture()`, which the RD renderer overrides and other renderers answer with an invalid RID. (Previously it printed "Texture storage not setup" on non-RD renderers; now it returns an invalid RID silently.)
- OpenXR's OpenGL extension creates and frees swapchain textures through `RSG::texture_storage` (the same object) instead of `GLES3::TextureStorage`, and includes `platform_gl.h` directly instead of relying on GLES3 headers for GL declarations.
- Remove unused RD renderer includes from the OpenXR Vulkan extension.

Tested: Linux and Windows builds. Not tested with XR hardware.

(Each PR description ends with the Claude Code attribution line when submitted.)
