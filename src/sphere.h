#ifndef SPHERE_H
#define SPHERE_H

#include "Shape.h"
#include "Vec3.h"


class sphere : public shape
{

  bool hit(const ray &r, double t_min, double t_max, HitRecord)
    vec3 oc = center -
}