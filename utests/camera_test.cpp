// test_camera.cpp
#include <iostream>
#include "Pcamera.h"

int main()
{
  camera *cam = new perspective_camera();

  ray r1 = cam->get_ray(0.0, 0.0);// should point toward lower-left
  ray r2 = cam->get_ray(0.5, 0.5);// should point roughly through center
  ray r3 = cam->get_ray(1.0, 1.0);// should point toward upper-right

  std::cout << "r1 dir: " << r1.direction() << "\n";
  std::cout << "r2 dir: " << r2.direction() << "\n";
  std::cout << "r3 dir: " << r3.direction() << "\n";

  delete cam;
  return 0;
}