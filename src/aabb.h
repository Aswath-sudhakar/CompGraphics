#ifndef AABB_H
#define AABB_H

#include <algorithm>
#include <cmath>
#include "Ray.h"
#include "Vec3.h"

class aabb
{
public:
  aabb() = default;
  aabb(const point3 &minimum, const point3 &maximum) : minimum_(minimum), maximum_(maximum) {}

  const point3 &minimum() const { return minimum_; }
  const point3 &maximum() const { return maximum_; }

  bool hit(const ray &r, double t_min, double t_max) const
  {
    for (int axis = 0; axis < 3; ++axis) {
      const double direction = r.direction()[axis];
      if (std::abs(direction) < 1e-12) {
        if (r.origin()[axis] < minimum()[axis] || r.origin()[axis] > maximum()[axis]) return false;
        continue;
      }
      const double inv_direction = 1.0 / direction;
      double origin = r.origin()[axis];
      double t0 = (minimum()[axis] - origin) * inv_direction;
      double t1 = (maximum()[axis] - origin) * inv_direction;
      if (inv_direction < 0.0) std::swap(t0, t1);
      t_min = std::max(t_min, t0);
      t_max = std::min(t_max, t1);
      if (t_max <= t_min) return false;
    }
    return true;
  }

  static aabb surrounding(const aabb &a, const aabb &b)
  {
    point3 minimum(
      std::min(a.minimum().x(), b.minimum().x()),
      std::min(a.minimum().y(), b.minimum().y()),
      std::min(a.minimum().z(), b.minimum().z()));
    point3 maximum(
      std::max(a.maximum().x(), b.maximum().x()),
      std::max(a.maximum().y(), b.maximum().y()),
      std::max(a.maximum().z(), b.maximum().z()));
    return aabb(minimum, maximum);
  }

private:
  point3 minimum_{ 0, 0, 0 };
  point3 maximum_{ 0, 0, 0 };
};

#endif
