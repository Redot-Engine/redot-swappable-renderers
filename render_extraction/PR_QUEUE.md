# PR Queue

Fixes found during extraction, batched for hand-testing before submission.
Each PR is its own branch off `master`, containing only the fix (no notes).
Nothing is pushed until the batch is hand-tested and approved. PRs go from the
fork only, never `Redot-Engine/redot-engine`.

| # | Branch | Summary | How to test by hand | Status |
|---|--------|---------|---------------------|--------|
| 1 | `fix/unused-renderer-scene-includes` | Remove unused `scene/` includes from renderer (EVIDENCE #1-3) | Build editor (`scons platform=linuxbsd target=editor`), open any project, confirm 3D and 2D scenes render. | Ready for testing |
