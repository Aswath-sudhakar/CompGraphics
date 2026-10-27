#include "frame_buffer.h"
#include "Pcamera.h"
#include "Vec3.h"
#include "shape_list.h"
#include "sphere.h"
#include "triangle.h"
#include <iostream>
#include <memory>
#include <vector>

int main()
{
  Framebuffer fb(500, 500);

  perspective_camera cam(point3(0, 3, 3), vec3(0, -0.4, -1), 0.5, 0.5, 0.4);

  std::vector<point_light> lights = {
    { point3(-5, 8, 5), vec3(0.7, 0.6, 0.45) },
    { point3(6, 6, 2), vec3(0.35, 0.45, 0.7) }
  };

  vec3 grey(.6, .6, .6), white(1, 1, 1);
  shape_list scene;


  scene.add(std::make_shared<triangle>(point3(-50, 0, 50), point3(50, 0, 50), point3(50, 0, -50), grey, LAMBERT));
  scene.add(std::make_shared<triangle>(point3(-50, 0, 50), point3(50, 0, -50), point3(-50, 0, -50), grey, LAMBERT));
  scene.add(std::make_shared<triangle>(point3(-4, 0, -6), point3(4, 0, -6), point3(4, 4, -6), white, MIRROR));
  scene.add(std::make_shared<triangle>(point3(-4, 0, -6), point3(4, 4, -6), point3(-4, 4, -6), white, MIRROR));
  scene.add(std::make_shared<triangle>(point3(0.9, 1.4, -0.6), point3(1.9, 1.4, -0.6), point3(1.4, 2.2, -1.1), vec3(.9, .8, .1), LAMBERT));


  scene.add(std::make_shared<sphere>(point3(-2, 0.8, -3), 0.8, vec3(.2, .8, .2), LAMBERT));// green
  scene.add(std::make_shared<sphere>(point3(-1.1, 0.6, -1.2), 0.6, vec3(.8, .1, .1), BLINN, 1.0, 64.0));// red
  scene.add(std::make_shared<sphere>(point3(0, 0.9, -3.5), 0.9, white, NORMAL));// normal
  scene.add(std::make_shared<sphere>(point3(0.3, 0.45, -1.6), 0.45, vec3(.1, .2, .8), BLINN, 1.0, 16.0));// blue
  scene.add(std::make_shared<sphere>(point3(2, 1, -3), 1.0, white, MIRROR));// mirror
  scene.add(std::make_shared<sphere>(point3(1.5, 0.5, 0.3), 0.5, white, MIRROR));// front mirror
  scene.add(std::make_shared<sphere>(point3(-1.3, 0.45, 0.4), 0.45, vec3(.2, .7, .7), DIFFUSE));// teal


  scene.add(std::make_shared<sphere>(point3(0.3, 1, 5), 1.0, vec3(.95, .5, .1), BLINN, 1.0, 32.0));// orange
  scene.add(std::make_shared<sphere>(point3(2.4, 0.8, 4.8), 0.8, vec3(.6, .2, .8), LAMBERT));// purple

  fb.render(cam, scene, lights, vec3(.5, .7, 1.0), 10);// 10 x 10 = 100 samples per pixel
  fb.exportAsPNG("scene.png");

  std::cout << "Done! Wrote scene.png" << std::endl;
  return 0;
}