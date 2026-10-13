#include "vec3.h"
#include "frame_buffer.h"
#include "png++/png.hpp"
#include <limits>
#include <iostream>
#include "shape_list.h"
#include <algorithm>
#include <cmath>
#include <random>

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
};


static vec3 shadeLambert(const HitRecord &rec, const shape &world, const point3 &lightPos, double ambient)
{
  vec3 toLight = lightPos - rec.p;
  double diffuse = std::max(0.0, dot(rec.normal, unit_vector(toLight)));

  HitRecord blocker;
  if (world.hit(ray(rec.p, toLight), 0.001, 1.0, blocker)) diffuse = 0.0;

  return (ambient + (1.0 - ambient) * diffuse) * rec.color;
}


static vec3 shadeBlinn(const ray &r, const HitRecord &rec, const shape &world, const point3 &lightPos, double ambient)
{
  vec3 N = unit_vector(rec.normal);
  vec3 toLight = lightPos - rec.p;
  vec3 L = unit_vector(toLight);
  vec3 V = unit_vector(-r.direction());
  vec3 H = unit_vector(L + V);

  double diffuse = std::max(0.0, dot(N, L));
  double specular = 0.0;
  if (diffuse > 0.0)
    specular = std::pow(std::max(0.0, dot(N, H)), rec.shininess);

  HitRecord blocker;
  if (world.hit(ray(rec.p, toLight), 0.001, 1.0, blocker)) {
    diffuse = 0.0;
    specular = 0.0;
  }

  vec3 c = (ambient + (1.0 - ambient) * diffuse) * rec.color
           + (rec.specStrength * specular) * vec3(1, 1, 1);
  return vec3(std::min(1.0, c.x()), std::min(1.0, c.y()), std::min(1.0, c.z()));
}

static vec3 reflect(const vec3 &v, const vec3 &n)
{
  return v - 2.0 * dot(v, n) * n;
}

static vec3 rayColor(const ray &r, const shape &world, const point3 &lightPos, const vec3 &bgColor, int depth, double ambient = 0.1)
{
  if (depth <= 0) return vec3(0, 0, 0);

  HitRecord rec;
  if (!world.hit(r, 0.001, std::numeric_limits<double>::infinity(), rec))
    return bgColor;


  vec3 local = (rec.shading == BLINN)
                 ? shadeBlinn(r, rec, world, lightPos, ambient)
                 : shadeLambert(rec, world, lightPos, ambient);

  if (rec.reflectivity <= 0.0) return local;

  vec3 n = unit_vector(rec.normal);
  if (dot(r.direction(), n) > 0) n = -n;

  ray reflected(rec.p, reflect(unit_vector(r.direction()), n));
  vec3 mirrorColor = rayColor(reflected, world, lightPos, bgColor, depth - 1, ambient);


  return (1.0 - rec.reflectivity) * local + rec.reflectivity * mirrorColor;
}

void Framebuffer::render(const perspective_camera &cam, const shape &world, const point3 &lightPos, const vec3 &bgColor, int n)
{

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      vec3 total(0, 0, 0);

      for (int j = 0; j < n; ++j) {
        for (int i = 0; i < n; ++i) {

          double u = (x + (i + randomDouble()) / n) / width;
          double v = 1.0 - (y + (j + randomDouble()) / n) / height;

          total += rayColor(cam.get_ray(u, v), world, lightPos, bgColor, 5);
        }
      }
      fbStorage[y * width + x] = total / (n * n);
    }
  }
};

void Framebuffer::LambShader(const perspective_camera &cam, const shape &s, const point3 &lightPos, const vec3 &bgColor, double ambient)
{
  const double inf = std::numeric_limits<double>::infinity();

  for (int y = 0; y < height; ++y) {
    double v = 1.0 - (y + 0.5) / height;
    for (int x = 0; x < width; ++x) {
      double u = (x + 0.5) / width;

      ray r = cam.get_ray(u, v);
      HitRecord rec;

      if (!s.hit(r, 0.001, inf, rec)) {
        fbStorage[y * width + x] = bgColor;
        continue;
      }
      if (rec.shading != LAMBERT) continue;

      fbStorage[y * width + x] = shadeLambert(rec, s, lightPos, ambient);
    }
  }
}

void Framebuffer::BlinnShader(const perspective_camera &cam, const shape &s, const point3 &lightPos, const vec3 &bgColor, double ambient)
{
  const double inf = std::numeric_limits<double>::infinity();

  for (int y = 0; y < height; ++y) {
    double v = 1.0 - (y + 0.5) / height;
    for (int x = 0; x < width; ++x) {
      double u = (x + 0.5) / width;

      ray r = cam.get_ray(u, v);
      HitRecord rec;

      if (!s.hit(r, 0.001, inf, rec)) {
        fbStorage[y * width + x] = bgColor;
        continue;
      }
      if (rec.shading != BLINN) continue;

      fbStorage[y * width + x] = shadeBlinn(r, rec, s, lightPos, ambient);
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