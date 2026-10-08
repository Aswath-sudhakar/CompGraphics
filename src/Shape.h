#ifndef SHAPE_H

#define SHAPE_H

#include "Vec3.h"
#include "Ray.h"

enum Shading { LAMBERT,
  BLINN };
struct HitRecord
{
  point3 p;
  vec3 normal;
  double t;
  vec3 color;
  Shading shading;
  double specStrength;
  double shininess;
};

class shape
{
public:
  shape(const vec3 &c = vec3(1, 1, 1), double spec = 0.0, double shine = 32.0) : color(c), specStrength(spec), shininess(shine) {}
  virtual ~shape() = default;
  virtual bool hit(const ray &r, double t_min, double t_max, HitRecord &rec) const = 0;


protected:
  vec3 color;
  double specStrength;
  double shininess;
};

#endif
