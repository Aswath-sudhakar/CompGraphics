#ifndef SHAPE_H

#define SHAPE_H

#include "Vec3.h"
#include "Ray.h"

struct HitRecord
{
  point3 p;
  vec3 normal;
  double t;
};

class shape
{
public:
  virtual ~shape() = default;
  virtual bool hit(const ray &r, double t_min, double t_max, HitRecord &rec) const = 0;
};

#endif
