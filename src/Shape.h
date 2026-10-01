#ifndef SHAPE_H

#define SHAPE_H

#include "Vec3.h"
#include "Ray.h"

struct HitRecord
{
  point3 p;
  vec3 normal;
  double t;
  vec3 color;
};

class shape
{
public:
  shape(const vec3 &c = vec3(1, 1, 1)) : color(c) {}
  virtual ~shape() = default;
  virtual bool hit(const ray &r, double t_min, double t_max, HitRecord &rec) const = 0;


protected:
  vec3 color;
};

#endif
