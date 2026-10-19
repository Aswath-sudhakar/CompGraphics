#ifndef SHAPE_H

#define SHAPE_H

#pragma once

#include "Vec3.h"
#include "Ray.h"

enum Shading {
  LAMBERT,
  BLINN,
  MIRROR,
  NORMAL,
  DIFFUSE
};

struct HitRecord
{
  point3 p;
  vec3 normal;
  double t;
  vec3 color;
  Shading shading;
  double specStrength;
  double shininess;
  double reflectivity;
};

class shape
{
public:
  shape(const vec3 &c = vec3(1, 1, 1), Shading m = LAMBERT, double spec = 0.5, double shine = 32.0, double refl = 0.0)
    : color(c), model(m), specStrength(spec), shininess(shine), reflectivity(refl) {}

  virtual ~shape() = default;
  virtual bool hit(const ray &r, double t_min, double t_max, HitRecord &rec) const = 0;

protected:
  vec3 color;
  Shading model;
  double specStrength;
  double shininess;
  double reflectivity;
};
#endif