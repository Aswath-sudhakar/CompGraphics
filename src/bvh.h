#ifndef BVH_H
#define BVH_H

#include <algorithm>
#include <memory>
#include <vector>

#include "shape.h"

class bvh_node : public shape
{
public:
  bvh_node(std::vector<std::shared_ptr<shape>> objects)
    : bvh_node(objects, 0, objects.size()) {}

  bool hit(const ray &r, double t_min, double t_max, HitRecord &rec) const override
  {
    if (!box.hit(r, t_min, t_max)) return false;
    if (leaf) return object->hit(r, t_min, t_max, rec);

    HitRecord left_rec, right_rec;
    bool hit_left = left && left->hit(r, t_min, t_max, left_rec);
    double right_max = hit_left ? left_rec.t : t_max;
    bool hit_right = right && right->hit(r, t_min, right_max, right_rec);
    if (hit_right) { rec = right_rec; return true; }
    if (hit_left) { rec = left_rec; return true; }
    return false;
  }

  aabb bounding_box() const override { return box; }

private:
  aabb box;
  std::shared_ptr<shape> object;
  std::shared_ptr<bvh_node> left;
  std::shared_ptr<bvh_node> right;
  bool leaf = false;

  bvh_node(std::vector<std::shared_ptr<shape>> &objects, size_t start, size_t end)
  {
    box = objects[start]->bounding_box();
    for (size_t index = start + 1; index < end; ++index)
      box = aabb::surrounding(box, objects[index]->bounding_box());

    const size_t count = end - start;
    if (count == 1) {
      leaf = true;
      object = objects[start];
      return;
    }

    vec3 extent = box.maximum() - box.minimum();
    int axis = extent.x() > extent.y() ? 0 : 1;
    if (extent.z() > extent[axis]) axis = 2;

    std::sort(objects.begin() + start, objects.begin() + end, [axis](const std::shared_ptr<shape> &a, const std::shared_ptr<shape> &b) {
      aabb aa = a->bounding_box();
      aabb bb = b->bounding_box();
      point3 ac = (aa.minimum() + aa.maximum()) / 2.0;
      point3 bc = (bb.minimum() + bb.maximum()) / 2.0;
      return ac[axis] < bc[axis];
    });

    const size_t middle = start + count / 2;
    left = std::make_shared<bvh_node>(bvh_node(objects, start, middle));
    right = std::make_shared<bvh_node>(bvh_node(objects, middle, end));
  }
};

#endif
