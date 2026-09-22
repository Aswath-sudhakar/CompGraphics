#ifndef CAMERA_H
#define CAMERA_H

#include "Ray.h"

class camera
{

public:
  virtual ~camera() = default;


  virtual ray get_ray(double u, double v) const = 0;
};

#endif