# Regression test

Renders a small 3D + 2D scene with each renderer/driver combination and writes
PNG frames, so two engine builds can be compared pixel by pixel.

```sh
./rtest.sh /mnt/disk2/Redot/26.3/bin/redot.linuxbsd.editor.x86_64 /tmp/out_old
./rtest.sh ../../bin/redot.linuxbsd.editor.x86_64 /tmp/out_new
for c in forward_plus_vulkan mobile_vulkan gl_compatibility_opengl3 gl_compatibility_opengl3_es; do
  compare -metric AE /tmp/out_old/${c}00000005.png /tmp/out_new/${c}00000005.png null:; echo " $c"
done
```

Expected: 0 differing pixels for each combination.
