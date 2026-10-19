#include "vec3.h"
#include "frame_buffer.h"
#include "png++/png.hpp"
#include <limits>
#include <iostream>
#include "shape_list.h"
#include <algorithm>
#include <cmath>
#include <random>
#include "light.h"

Framebuffer::Framebuffer(int w, int h)
{
  width = w;
  height = h;
  fbStorage.resize(width * height);
}


static double randomDouble()
{
  static std::mt19937 gen(12345);
  static std::uniform_real_distribution<double> dist(0.0, 1.0);
  return dist(gen);
}

static vec3 randomUnitVector()
{
  while (true) {
    vec3 p(2 * randomDouble() - 1, 2 * randomDouble() - 1, 2 * randomDouble() - 1);
    double len2 = p.length_squared();
    if (len2 > 1e-12 && len2 <= 1.0) return p / std::sqrt(len2);
  }
}

static vec3 reflect(const vec3 &v, const vec3 &n)
{
  return v - 2.0 * dot(v, n) * n;
}

static vec3 clamp01(const vec3 &c)
{
  return vec3(std::min(1.0, c.x()), std::min(1.0, c.y()), std::min(1.0, c.z()));
}


static bool inShadow(const shape &world, const point3 &p, const vec3 &toLight)
{
  HitRecord blocker;
  return world.hit(ray(p, toLight), 0.001, 1.0, blocker);
}

static vec3 shadeLambert(const HitRecord &rec, const shape &world, const std::vector<point_light> &lights, double ambient)
{
  vec3 sum(ambient, ambient, ambient);
  for (const auto &L : lights) {
    vec3 toLight = L.pos - rec.p;
    if (inShadow(world, rec.p, toLight)) continue;
    sum += std::max(0.0, dot(rec.normal, unit_vector(toLight))) * L.intensity;
  }
  return clamp01(rec.color * sum);
}

static vec3 shadeBlinn(const ray &r, const HitRecord &rec, const shape &world, const std::vector<point_light> &lights, double ambient)
{
  vec3 V = unit_vector(-r.direction());
  vec3 diff(ambient, ambient, ambient);
  vec3 spec(0, 0, 0);

  for (const auto &L : lights) {
    vec3 toLight = L.pos - rec.p;
    if (inShadow(world, rec.p, toLight)) continue;

    vec3 Ld = unit_vector(toLight);
    double nl = dot(rec.normal, Ld);
    if (nl <= 0.0) continue;

    diff += nl * L.intensity;
    vec3 H = unit_vector(Ld + V);
    spec += std::pow(std::max(0.0, dot(rec.normal, H)), rec.shininess) * L.intensity;
  }
  return clamp01(rec.color * diff + rec.specStrength * spec);
}

static vec3 rayColor(const ray &r, const shape &world, const std::vector<point_light> &lights, const vec3 &bgColor, int depth)
{
  if (depth <= 0) return vec3(0, 0, 0);

  HitRecord rec;
  if (!world.hit(r, 0.001, std::numeric_limits<double>::infinity(), rec))
    return bgColor;

  rec.normal = unit_vector(rec.normal);
  if (dot(r.direction(), rec.normal) > 0) rec.normal = -rec.normal;

  const double ambient = 0.0;

  switch (rec.shading) {
  case BLINN:
    return shadeBlinn(r, rec, world, lights, ambient);

  case MIRROR: {
    ray refl(rec.p, reflect(unit_vector(r.direction()), rec.normal));
    return rec.color * rayColor(refl, world, lights, bgColor, depth - 1);
  }

  case NORMAL:
    return 0.5 * (rec.normal + vec3(1, 1, 1));

  case DIFFUSE: {
    vec3 dir = rec.normal + randomUnitVector();
    if (dir.length_squared() < 1e-12) dir = rec.normal;
    return rec.color * rayColor(ray(rec.p, dir), world, lights, bgColor, depth - 1);
  }

  default:
    return shadeLambert(rec, world, lights, ambient);
  }
}

void Framebuffer::render(const perspective_camera &cam, const shape &world, const std::vector<point_light> &lights, const vec3 &bgColor, int n)
{
  const int maxDepth = 5;

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      vec3 total(0, 0, 0);

      for (int j = 0; j < n; ++j) {
        for (int i = 0; i < n; ++i) {
          double u = (x + (i + randomDouble()) / n) / width;
          double v = 1.0 - (y + (j + randomDouble()) / n) / height;
          total += rayColor(cam.get_ray(u, v), world, lights, bgColor, maxDepth);
        }
      }
      fbStorage[y * width + x] = total / (n * n);
    }
  }
}


void Framebuffer::shapeShader(const perspective_camera &cam, const shape &s, const vec3 &bgColor)
{
  for (int y = 0; y < height; ++y) {
    double v = 1.0 - (static_cast<double>(y) + 0.5) / height;

    for (int x = 0; x < width; ++x) {
      double u = (static_cast<double>(x) + 0.5) / width;

      ray r = cam.get_ray(u, v);
      HitRecord rec;

      bool hit = s.hit(r, 0.001, std::numeric_limits<double>::infinity(), rec);
      fbStorage[y * width + x] = hit ? rec.color : bgColor;
    }
  }
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