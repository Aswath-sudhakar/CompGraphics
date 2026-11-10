In order to run this project and test the files, first run cmake --build build VCPkg, then run .\buildVCPkg\utests\Debug\vec3_test.exe to run the test file in the terminal. For the camera and the ray test, run the previous command but with the other exe names 

for the json loader run .\buildVCPkg\examples\Debug\render_json.exe examples\basic_scene.json render.png 800 600 in the command and then check the output.png in the root file structure of the project
## Rendering JSON scenes

Build the `render_json` CMake target, then run it with a scene file:

```powershell
.\buildVCPkg\examples\Debug\render_json.exe examples\basic_scene.json render.png 800 600
```

The output path and dimensions are optional; defaults are `render.png`, 800 pixels wide, and 600 pixels high. JSON scenes can specify a perspective camera, point or area lights, Lambertian/Phong/mirror shaders, and sphere, triangle, box, or OBJ mesh shapes. Shape transforms are applied; spheres support uniform scaling. Area lights are approximated as point lights, and textures and environment maps are ignored with warnings because the renderer does not implement them.

### Acceleration mode comparison

The renderer uses linear shape traversal by default, preserving the original behavior. Pass `--bvh` to build and use a bounding volume hierarchy instead:

```powershell
.\buildVCPkg\examples\Debug\render_json.exe examples\basic_scene.json linear.png 800 600 --no-bvh
.\buildVCPkg\examples\Debug\render_json.exe examples\basic_scene.json bvh.png 800 600 --bvh
```

The render reports only the ray-tracing elapsed time so the two modes can be compared easily. The PNG output should otherwise be equivalent.

### 1000-object benchmarks

Two 1000-object, 1000 x 1000 scenes are included for timing comparisons:

- `examples/bvh_benchmark_grid_1000.json`: 1,000 spheres in a spatial grid, plus a ground box.
- `examples/bvh_benchmark_cluster_1000.json`: 1,000 clustered spheres, plus a ground box.

```powershell
# Spatial grid benchmark
.\buildVCPkg\examples\Release\render_json.exe examples\bvh_benchmark_grid_1000.json grid-linear.png 1000 1000 --no-bvh
.\buildVCPkg\examples\Release\render_json.exe examples\bvh_benchmark_grid_1000.json grid-bvh.png 1000 1000 --bvh

# Clustered benchmark
.\buildVCPkg\examples\Release\render_json.exe examples\bvh_benchmark_cluster_1000.json cluster-linear.png 1000 1000 --no-bvh
.\buildVCPkg\examples\Release\render_json.exe examples\bvh_benchmark_cluster_1000.json cluster-bvh.png 1000 1000 --bvh
```

for the bvh run: 
.\buildVCPkg\examples\Release\render_json.exe examples\bvh_benchmark_grid_1000.json grid-linear.png 1000 1000 --no-bvh
.\buildVCPkg\examples\Release\render_json.exe examples\bvh_benchmark_grid_1000.json grid-bvh.png 1000 1000 --bvh


.\buildVCPkg\examples\Release\render_json.exe examples\bvh_benchmark_cluster_1000.json cluster-linear.png 1000 1000 --no-bvh
.\buildVCPkg\examples\Release\render_json.exe examples\bvh_benchmark_cluster_1000.json cluster-bvh.png 1000 1000 --bvh
