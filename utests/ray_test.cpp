#include <iostream>
#include "Ray.h"

int main()
{
  point3 origin(0, 0, 0);
  vec3 direction(1, 2, 3);

  ray r(origin, direction);

  std::cout << "Origin:" << r.origin() << "\n";
  std::cout << "Direction:" << r.direction() << "\n";

  for (double t = 0.0; t <= 2.0; t += 0.5) {
    point3 p = r.at(t);
    std::cout << "t=" << t << " -> " << p << "\n";
  }
  if (r.at(0.0).x() == origin.x() && r.at(0.0).y() == origin.y() && r.at(0.0).z() == origin.z()) {
    std::cout << "PASS: at(0) == origin\n";
  } else {
    std::cout << "FAIL: at(0) != origin\n";
  }

  return 0;
}