#ifndef SHAPE_H

#define SHAPE_H

#include "Vec3.h"
#include "Ray.h"

struct HitReord
{
  point3 p;
  vec3 normal;
  double t;
};

class shape
{

  virtual bool hit(const ray &r, double t_min, double t_max, HitReord) = 0;
};

#endif
