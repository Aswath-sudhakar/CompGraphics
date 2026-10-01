#include <iostream>
#include <limits>
#include "Ray.h"
#include "Sphere.h"
#include "Pcamera.h"

void report(bool ok, const char *msg)
{
  std::cout << (ok ? "PASS: " : "FAIL: ") << msg << "\n";
}

int main()
{
  const double inf = std::numeric_limits<double>::infinity();

  point3 center(0, 0, -1);
  double radius = 0.5;
  sphere s(center, radius, vec3(1, 0, 0));// drop the color arg if your constructor has none
  HitRecord rec;

  // 1. ray straight at the center
  ray r1(point3(0, 0, 0), vec3(0, 0, -1));
  bool h1 = s.hit(r1, 0.001, inf, rec);
  std::cout << "Center ray: hit=" << h1 << " t=" << rec.t
            << " p=" << rec.p << " normal=" << rec.normal << "\n";
  report(h1, "ray at center hits");
  report(std::abs(rec.t - 0.5) < 1e-9, "hit distance t == 0.5");
  report(std::abs(rec.p.z() - (-0.5)) < 1e-9, "hit point is front of sphere (z == -0.5)");
  report(std::abs(rec.normal.z() - 1.0) < 1e-9, "normal points back at camera");
  report(std::abs(rec.normal.length() - 1.0) < 1e-9, "normal is unit length");

  // 2. ray pointing away
  ray r2(point3(0, 0, 0), vec3(0, 1, 0));
  bool h2 = s.hit(r2, 0.001, inf, rec);
  std::cout << "Away ray: hit=" << h2 << "\n";
  report(!h2, "ray pointing away misses");

  // 3. ray just inside and just outside the radius
  ray inside(point3(0.4, 0, 0), vec3(0, 0, -1));
  ray outside(point3(0.6, 0, 0), vec3(0, 0, -1));
  bool hin = s.hit(inside, 0.001, inf, rec);
  bool hout = s.hit(outside, 0.001, inf, rec);
  std::cout << "Offset 0.4: hit=" << hin << "  Offset 0.6: hit=" << hout << "\n";
  report(hin, "ray offset 0.4 (inside radius) hits");
  report(!hout, "ray offset 0.6 (outside radius) misses");

  // 4. t_max too small (real hit is at t = 0.5)
  bool h4 = s.hit(r1, 0.001, 0.4, rec);
  std::cout << "t_max 0.4: hit=" << h4 << "\n";
  report(!h4, "hit beyond t_max is rejected");

  // 5. sphere behind the ray
  ray r5(point3(0, 0, 0), vec3(0, 0, 1));
  bool h5 = s.hit(r5, 0.001, inf, rec);
  std::cout << "Behind ray: hit=" << h5 << "\n";
  report(!h5, "sphere behind the ray is not hit");

  // 6. ray starting inside the sphere
  ray r6(center, vec3(0, 0, -1));
  bool h6 = s.hit(r6, 0.001, inf, rec);
  std::cout << "Inside ray: hit=" << h6 << " t=" << rec.t << "\n";
  report(h6 && std::abs(rec.t - 0.5) < 1e-9, "ray from inside hits far side at t == 0.5");

  // 7. through the camera: center pixel hits, corners miss
  perspective_camera cam(point3(0, 0, 0), vec3(0, 0, -1), 2.0 * 16.0 / 9.0, 2.0, 1.0);
  bool hc = s.hit(cam.get_ray(0.5, 0.5), 0.001, inf, rec);
  bool hk1 = s.hit(cam.get_ray(0.0, 0.0), 0.001, inf, rec);
  bool hk2 = s.hit(cam.get_ray(1.0, 1.0), 0.001, inf, rec);
  std::cout << "Camera: center=" << hc << " corner(0,0)=" << hk1 << " corner(1,1)=" << hk2 << "\n";
  report(hc, "camera center ray hits");
  report(!hk1 && !hk2, "camera corner rays miss");

  return 0;
}