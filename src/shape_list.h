#ifndef SHAPE_LIST_H
#define SHAPE_LIST_H

#include <memory>
#include <vector>
#include "shape.h"
#include "bvh.h"

class shape_list : public shape
{
public:
  void add(std::shared_ptr<shape> s) { objects.push_back(s); }

  void use_bvh(bool enabled = true)
  {
    bvh_enabled = enabled;
    bvh.reset();
    if (enabled && !objects.empty()) bvh = std::make_shared<bvh_node>(objects);
  }

  bool hit(const ray &r, double t_min, double t_max, HitRecord &rec) const override
  {
    if (bvh_enabled && bvh) return bvh->hit(r, t_min, t_max, rec);

    HitRecord temp;
    bool hit_any = false;
    double closest = t_max;

    for (const auto &obj : objects) {
      if (obj->hit(r, t_min, closest, temp)) {
        hit_any = true;
        closest = temp.t;
        rec = temp;
      }
    }
    return hit_any;
  }

  aabb bounding_box() const override
  {
    if (objects.empty()) return aabb(point3(0, 0, 0), point3(0, 0, 0));
    aabb box = objects.front()->bounding_box();
    for (size_t i = 1; i < objects.size(); ++i)
      box = aabb::surrounding(box, objects[i]->bounding_box());
    return box;
  }

private:
  std::vector<std::shared_ptr<shape>> objects;
  bool bvh_enabled = false;
  std::shared_ptr<bvh_node> bvh;
};

#endif