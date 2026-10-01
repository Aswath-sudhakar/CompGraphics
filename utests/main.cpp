#include "frame_buffer.h"
#include "Pcamera.h"
#include "Vec3.h"
#include "sphere.h"
#include <iostream>

int main()
{
  std::cout << "Starting render..." << std::endl;

  Framebuffer fb(1600, 900);


  perspective_camera cam(point3(0, 0, 0), vec3(0, 0, -1), 2.0 * 16.0 / 9.0, 2.0, 1.0);


  sphere disc(point3(0, 0, -1), 0.53, vec3(188 / 255.0, 0.0, 45 / 255.0));

  fb.shapeShader(cam, disc, vec3(1, 1, 1));// white background
  fb.exportAsPNG("flag.png");

  std::cout << "Done! Wrote flag.png" << std::endl;
  return 0;
}