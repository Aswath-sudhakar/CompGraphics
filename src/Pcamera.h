#ifndef PERSPECTIVE_CAMERA_H
#define PERSPECTIVE_CAMERA_H

#include "camera.h"

class perspective_camera : public camera
{
public:
  perspective_camera(const point3 &cameraPos, const vec3 &viewDir, double planeWidth, double planeHeight, double focalLength)
  {
    origin = cameraPos;

    vec3 worldUp(0, 1, 0);
    w = unit_vector(viewDir);
    u = unit_vector(cross(w, worldUp));
    v = cross(u, w);

    horizontal = planeWidth * u;
    vertical = planeHeight * v;
    lower_left_corner = origin + focalLength * w - horizontal / 2 - vertical / 2;
  }

  ray get_ray(double s, double t) const override
  {
    return ray(origin, lower_left_corner + s * horizontal + t * vertical - origin);
  }

private:
  point3 origin;
  vec3 lower_left_corner;
  vec3 horizontal;
  vec3 vertical;
  vec3 u, v, w;
};

#endif