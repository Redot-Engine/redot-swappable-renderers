#!/usr/bin/env python3
"""Report engine code that includes built-in renderer backend headers.

The built-in backends (RD Forward+/Mobile, GLES3 Compatibility, Dummy) must only
be reachable through RendererRegistry. Anything outside the backend directories
that includes their headers is a boundary violation, unless listed in ALLOWED.

Usage: check_renderer_boundary.py [repo_root]   (exit code 1 if violations)
"""

import os
import re
import sys

# servers/rendering/dummy/ is not a backend: it's the null implementation of every
# subsystem, needed for headless mode and usable as a base class for new renderers.
BACKENDS = {
    "renderer_rd": "servers/rendering/renderer_rd/",
    "gles3": "drivers/gles3/",
}

# File -> backends it may include, with the reason.
ALLOWED = {
    "drivers/register_driver_types.cpp": ({"renderer_rd", "gles3"}, "registers the built-in renderers"),
}

SCAN_DIRS = ["core", "drivers", "editor", "main", "modules", "platform", "scene", "servers", "tests"]
EXTS = (".h", ".hpp", ".cpp", ".mm", ".m", ".inc")
INCLUDE_RE = re.compile(r'^\s*#\s*(?:include|import)\s+"([^"]+)"', re.M)


def backend_of(path):
    for name, prefix in BACKENDS.items():
        if path.startswith(prefix):
            return name
    return None


def resolve(src, inc):
    # Engine includes are root-relative; fall back to file-relative.
    if os.path.exists(inc):
        return os.path.normpath(inc)
    rel = os.path.normpath(os.path.join(os.path.dirname(src), inc))
    return rel if os.path.exists(rel) else inc


def main():
    os.chdir(sys.argv[1] if len(sys.argv) > 1 else ".")
    violations = []
    for top in SCAN_DIRS:
        for dirpath, _, files in os.walk(top):
            for f in files:
                if not f.endswith(EXTS):
                    continue
                src = os.path.join(dirpath, f)
                if backend_of(src):
                    continue  # Backends may include themselves and each other's public pieces.
                text = open(src, encoding="utf-8", errors="replace").read()
                for m in INCLUDE_RE.finditer(text):
                    target = resolve(src, m.group(1))
                    b = backend_of(target)
                    if not b:
                        continue
                    allowed = ALLOWED.get(src, (set(), ""))[0]
                    if b in allowed:
                        continue
                    line = text.count("\n", 0, m.start()) + 1
                    violations.append((b, src, line, m.group(1)))
    violations.sort()
    by_backend = {}
    for b, src, line, inc in violations:
        by_backend.setdefault(b, []).append(f"  {src}:{line} -> {inc}")
    for b, lines in by_backend.items():
        print(f"[{b}] {len(lines)}")
        print("\n".join(lines))
    print(f"TOTAL {len(violations)} violation(s) in {len({v[1] for v in violations})} file(s)")
    return 1 if violations else 0


if __name__ == "__main__":
    sys.exit(main())
