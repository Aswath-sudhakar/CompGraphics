#include "frame_buffer.h"
#include "Pcamera.h"
#include "Vec3.h"
#include <iostream>

int main()
{
  std::cout << "Starting render..." << std::endl;

  Framebuffer fb(200, 200);

  // Non-default camera — pick your own distinct position/direction
  perspective_camera cam(
    point3(2.0, 1.0, 2.0),// camera position
    vec3(-2.0, -1.0, -3.0),// view direction
    2.0,// image plane width
    2.0,// image plane height
    1.0// focal length
  );

  std::cout << "Camera created, running shader..." << std::endl;

  fb.rayDirectionShader(cam);

  std::cout << "Shader complete, exporting PNG..." << std::endl;

  fb.exportAsPNG("output.png");

  std::cout << "Done! Wrote output.png" << std::endl;

  return 0;
}