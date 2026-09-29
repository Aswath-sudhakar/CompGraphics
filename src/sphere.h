#ifndef SPHERE_H
#define SPHERE_H

#include "Shape.h"
#include "Vec3.h"
#include <cmath>


class sphere : public shape
{
public:
  sphere(const point3 &c, double r) : center(c), radius(r) {}

  bool hit(const ray &r, double t_min, double t_max, HitRecord &rec) const override;
  {
    vec3 oc = r.origin() - center;
    double a = r.diretion().length_squared();
    double b_half = dot(oc, r.direction());
    double c = oc.length_squared() - radius * radius;

    double disc = b_half * b_half - a * c;
    if (disc < 0) return false;
  }

private:
  point3 center;
  double radius;
};


#endif