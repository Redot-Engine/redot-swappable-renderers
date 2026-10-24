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

## Feature-flag probe

`flagproj/` exercises every renderer-feature check reachable from script
(environment effects, DoF/auto-exposure, 3D scaling modes, SSAA/TAA,
`hint_normal_roughness_texture`, visual shader code generation) plus an editor
plugin that prints Environment glow property visibility.

```sh
./flagtest.sh /mnt/disk2/Redot/26.3/bin/redot.linuxbsd.editor.x86_64 /tmp/flag_old
./flagtest.sh ../../bin/redot.linuxbsd.editor.x86_64 /tmp/flag_new
for m in forward_plus mobile gl_compatibility dummy headless; do diff -q /tmp/flag_old/$m.txt /tmp/flag_new/$m.txt; done
```

Expected: no differences. (The editor plugin's glow check runs with `-e`; see the
commit that added feature flags for the command.)
