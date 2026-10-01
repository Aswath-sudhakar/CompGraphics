#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <cmath>
#include "shape.h"
#include "vec3.h"

class triangle : public shape
{
public:
  triangle(const point3 &a, const point3 &b, const point3 &c, const vec3 &col)
    : shape(col), v0(a), v1(b), v2(c) {}

  bool hit(const ray &r, double t_min, double t_max, HitRecord &rec) const override
  {
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;

    vec3 h = cross(r.direction(), edge2);
    double det = dot(edge1, h);
    if (std::fabs(det) < 1e-8) return false;

    double inv = 1.0 / det;
    vec3 s = r.origin() - v0;

    double u = inv * dot(s, h);
    if (u < 0.0 || u > 1.0) return false;

    vec3 q = cross(s, edge1);
    double v = inv * dot(r.direction(), q);
    if (v < 0.0 || u + v > 1.0) return false;

    double t = inv * dot(edge2, q);
    if (t < t_min || t > t_max) return false;

    rec.t = t;
    rec.p = r.at(t);
    rec.normal = unit_vector(cross(edge1, edge2));
    rec.color = color;
    return true;
  }

private:
  point3 v0, v1, v2;
};

#endif