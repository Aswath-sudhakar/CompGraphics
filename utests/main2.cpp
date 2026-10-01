#include "frame_buffer.h"
#include "Pcamera.h"
#include "Vec3.h"
#include "shape_list.h"
#include "sphere.h"
#include "triangle.h"
#include <iostream>
#include <memory>

int main()
{
  std::cout << "Starting render..." << std::endl;

  Framebuffer fb(1600, 900);

  perspective_camera cam(point3(0, 0, 0), vec3(0, 0, -1), 2.0 * 16.0 / 9.0, 2.0, 1.0);

  shape_list scene;
  scene.add(std::make_shared<triangle>(
    point3(-0.8, -0.6, -1.2), point3(0.8, -0.6, -1.2), point3(0.0, 0.8, -1.2), vec3(0.1, 0.4, 0.9)));// blue, farther
  scene.add(std::make_shared<sphere>(
    point3(0.3, 0.0, -1.0), 0.4, vec3(0.74, 0.0, 0.18)));// red, closer

  fb.shapeShader(cam, scene, vec3(1, 1, 1));
  fb.exportAsPNG("overlap.png");

  std::cout << "Done! Wrote overlap.png" << std::endl;
  return 0;
}