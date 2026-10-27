In order to run this project and test the files, first run cmake --build build VCPkg, then run .\buildVCPkg\utests\Debug\vec3_test.exe to run the test file in the terminal. For the camera and the ray test, run the previous command but with the other exe names 

for the json loader run .\buildVCPkg\examples\Debug\render_json.exe examples\basic_scene.json render.png 800 600 in the command and then check the output.png in the root file structure of the project
## Rendering JSON scenes

Build the `render_json` CMake target, then run it with a scene file:

```powershell
.\buildVCPkg\examples\Debug\render_json.exe examples\basic_scene.json render.png 800 600
```

The output path and dimensions are optional; defaults are `render.png`, 800 pixels wide, and 600 pixels high. JSON scenes can specify a perspective camera, point or area lights, Lambertian/Phong/mirror shaders, and sphere, triangle, box, or OBJ mesh shapes. Shape transforms are applied; spheres support uniform scaling. Area lights are approximated as point lights, and textures and environment maps are ignored with warnings because the renderer does not implement them.
