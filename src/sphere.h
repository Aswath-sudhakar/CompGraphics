#ifndef SPHERE_H
#define SPHERE_H

#include "Shape.h"
#include "Vec3.h"
#include <cmath>


class sphere : public shape
{
public:
  sphere(const point3 &c, double r, const vec3 &col, double spec = 0.0, double shine = 32.0)
    : shape(col, spec, shine), center(c), radius(r) {}

  bool hit(const ray &r, double t_min, double t_max, HitRecord &rec) const override
  {
    vec3 oc = r.origin() - center;
    double a = r.direction().length_squared();
    double b_half = dot(oc, r.direction());
    double c = oc.length_squared() - radius * radius;

    double disc = b_half * b_half - a * c;
    if (disc < 0) return false;

    double sqrtd = std::sqrt(disc);
    double root = (-b_half - sqrtd) / a;
    if (root < t_min || root > t_max) {

      root = (-b_half + sqrtd) / a;
      if (root < t_min || root > t_max) return false;
    }

    rec.t = root;
    rec.p = r.at(root);
    rec.normal = (rec.p - center) / radius;
    rec.color = color;
    rec.specStrength = specStrength;
    rec.shininess = shininess;
    return true;
  };

private:
  point3 center;
  double radius;
};


#endif