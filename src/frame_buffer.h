#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <iostream>
#include <vector>
#include "Vec3.h"
#include "Pcamera.h"
using namespace std;

class Framebuffer
{
public:
  Framebuffer() : width(0), height(0) {}
  Framebuffer(int w, int h);
  void clear(vec3 c);
  void exportAsPNG(string filename);
  vec3 lerp(const vec3 &c1, const vec3 &c2, double t);
  void gradientTB(const vec3 &c1, const vec3 &c2);
  void gradientLR(const vec3 &c1, const vec3 &c2);
  void colorArrayTB(vector<vec3> cArr);
  void rayDirectionShader(const perspective_camera &cam);

private:
  int width, height;
  double t;
  vector<vec3> fbStorage;
};

#endif//