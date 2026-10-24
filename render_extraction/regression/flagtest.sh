#!/bin/bash
# usage: flagtest.sh <binary> <outdir>
B=$1; O=$2; P=$(dirname "$0")/flagproj; mkdir -p "$O"
"$B" --headless --path "$P" --import >/dev/null 2>&1
for combo in forward_plus:vulkan mobile:vulkan gl_compatibility:opengl3 dummy:dummy; do
  m=${combo%%:*}; d=${combo##*:}
  timeout 60 "$B" --path "$P" --rendering-method $m --rendering-driver $d > "$O/$m.raw" 2>&1
  # Keep probe output and warnings/errors; drop version banner, device lines and source paths' line numbers.
  grep -E "PROBE|WARNING|ERROR|only|unavailable|not supported" "$O/$m.raw" | sed -E 's/\(([^)]*\.cpp):[0-9]+\)/(\1)/; s/^ +at: /at: /' > "$O/$m.txt"
done
timeout 60 "$B" --headless --path "$P" > "$O/headless.raw" 2>&1
grep -E "PROBE|WARNING|ERROR|only|unavailable|not supported" "$O/headless.raw" | sed -E 's/\(([^)]*\.cpp):[0-9]+\)/(\1)/; s/^ +at: /at: /' > "$O/headless.txt"
wc -l "$O"/*.txt
