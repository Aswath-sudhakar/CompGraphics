#include "frame_buffer.h"
#include "Pcamera.h"
#include "Vec3.h"
#include "shape_list.h"
#include "sphere.h"
#include <iostream>
#include <memory>

int main()
{
  std::cout << "Starting render..." << std::endl;

  Framebuffer fb(1600, 900);

  perspective_camera cam(point3(0, 0, 0), vec3(0, 0, -1), 2.0 * 16.0 / 9.0, 2.0, 1.0);

  shape_list scene;
  scene.add(std::make_shared<sphere>(
    point3(-0.6, 0.0, -1.5), 0.5, vec3(0.74, 0.0, 0.18), LAMBERT, 0.0, 32.0, 0.2));// left: slightly reflective
  scene.add(std::make_shared<sphere>(
    point3(0.6, 0.0, -1.5), 0.5, vec3(0.1, 0.4, 0.9), BLINN, 0.6, 64.0, 0.8));// right: very reflective

  fb.render(cam, scene, point3(-2, 2, 1), vec3(0.5, 0.7, 1.0), 4);
  fb.exportAsPNG("mirror_aa.png");

  std::cout << "Done! Wrote spheres.png" << std::endl;
  return 0;
}