#include "vec3.h"
#include "frame_buffer.h"
#include "png++/png.hpp"

#include <iostream>


Framebuffer::Framebuffer(int w, int h)
{
  width = w;
  height = h;
  fbStorage.resize(width * height);
}

void Framebuffer::rayDirectionShader(const perspective_camera &cam)
{
  for (int y = 0; y < height; ++y) {
    double pv = 1.0 - 2.0 * (static_cast<double>(y) + 0.5) / height;

    for (int x = 0; x < width; ++x) {
      double pu = 2.0 * (static_cast<double>(x) + 0.5) / width - 1.0;

      ray r = cam.get_ray(pu, pv);
      vec3 rayDir = unit_vector(r.direction());

      vec3 color(
        (rayDir.x() + 1.0) * 0.5,
        (rayDir.y() + 1.0) * 0.5,
        (rayDir.z() + 1.0) * 0.5);

      fbStorage[y * width + x] = color;
    }
  }
}

void Framebuffer::clear(vec3 c)
{
  for (int index = 0; index < fbStorage.size(); index++) {
    fbStorage[index] = c;
  }
}

vec3 Framebuffer::lerp(const vec3 &c1, const vec3 &c2, double t)
{
  vec3 c = (1 - t) * c1 + t * c2;
  return c;
}


void Framebuffer::gradientTB(const vec3 &c1, const vec3 &c2)
{
  for (size_t y = 0; y < height; ++y) {
    t = static_cast<double>(y) / height;
    for (size_t x = 0; x < width; ++x) {
      vec3 c = (1 - t) * c1 + t * c2;
      fbStorage[y * width + x] = lerp(c1, c2, t);
    }
  }
}

void Framebuffer::gradientLR(const vec3 &c1, const vec3 &c2)
{
  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      t = static_cast<double>(x) / width;
      fbStorage[y * width + x] = lerp(c1, c2, t);
    }
  }
}


void Framebuffer::colorArrayTB(vector<vec3> cArr)
{
  int h = height / cArr.size();
  for (size_t z = 0; z < cArr.size(); ++z) {
    vec3 c = cArr[z];
    for (size_t y = z * h; y < (z + 1) * h; ++y) {
      for (size_t x = 0; x < width; ++x) {
        fbStorage[y * width + x] = c;
      }
    }
  }
}


void Framebuffer::exportAsPNG(string filename)
{
  png::image<png::rgb_pixel> imData(width, height);
  for (size_t y = 0; y < imData.get_height(); ++y) {
    for (size_t x = 0; x < imData.get_width(); ++x) {
      imData[y][x] = png::rgb_pixel(
        int(255.999 * fbStorage[y * width + x].x()),
        int(255.999 * fbStorage[y * width + x].y()),
        int(255.999 * fbStorage[y * width + x].z()));
    }
  }
  imData.write(filename);
}