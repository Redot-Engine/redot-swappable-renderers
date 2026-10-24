#!/bin/bash
# usage: rtest.sh <binary> <outdir>
B=$1; O=$2; P=$(dirname "$0")/testproj
mkdir -p "$O"
"$B" --headless --path "$P" --import >/dev/null 2>&1
for combo in forward_plus:vulkan mobile:vulkan gl_compatibility:opengl3 gl_compatibility:opengl3_es; do
  m=${combo%%:*}; d=${combo##*:}
  timeout 60 "$B" --path "$P" --rendering-method $m --rendering-driver $d --write-movie "$O/${m}_${d}.png" --fixed-fps 10 --quit-after 6 > "$O/${m}_${d}.log" 2>&1
  echo "$m/$d exit=$? $(grep -m1 -oE '(Vulkan|OpenGL( ES)?) [0-9.]+[^-]*- [A-Za-z+ ]+' "$O/${m}_${d}.log")"
done
timeout 60 "$B" --headless --path "$P" --quit-after 6 > "$O/headless.log" 2>&1; echo "headless exit=$?"
timeout 60 "$B" --path "$P" --rendering-driver dummy --quit-after 6 > "$O/dummy.log" 2>&1; echo "dummy exit=$?"
