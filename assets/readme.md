# Shaders

Shaders are compiled to header files and included in CrossWindow-ImGui, if you want to recompile them run the compile script:

```bash
# DirectX 12
dxc -T vs_6_6 -Fh assets/imgui.vert.h assets/imgui.vert.hlsl
dxc -T ps_6_6 -Fh assets/imgui.frag.h assets/imgui.frag.hlsl
```