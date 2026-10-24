# Render Extraction

Working notes for extracting Redot's renderer into a standalone library behind a
swappable renderer interface.

## Goal

1. Define a stable renderer interface (derived from `RenderingServer`) that any
   renderer can implement.
2. Move the current renderer (`servers/rendering/`, `drivers/{vulkan,gles3,d3d12,metal}`)
   into a standalone library that implements it.
3. A second, independent renderer will later plug into the same interface.

## Plan

1. **Dependency map** - build with `compiledb=yes`, generate the include/dependency
   graph in both directions (renderer -> engine, engine -> renderer).
2. **Baseline** - clean build, tests, reference screenshots for regression checks.
3. **Incremental extraction** - one dependency at a time; every commit builds.
4. **Bug fixes** - separate branches/PRs to Redot-Engine/redot-engine.
5. **Evidence log** - see `EVIDENCE.md`.

## Status

- [x] Workspace created from `26.3` (`master` @ 860c27364f), branch `render-extraction`.
- [x] Dependency map - pass 1 (DEPENDENCY_MAP.md)
- [x] Regression harness (`regression/`), pixel-identical vs 26.3
- [x] Registry phase 1: `RendererRegistry`, all platforms + headless converted (Linux verified; other platforms not compile-tested)
- [ ] Registry phase 2: `CONTEXT_SELF_MANAGED` + raw Vulkan test renderer
- [ ] Remove old renderer includes from non-Linux platforms (needs cross-compile)
- [ ] Baseline
