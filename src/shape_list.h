#ifndef SHAPE_LIST_H
#define SHAPE_LIST_H

#include <memory>
#include <vector>
#include "shape.h"

class shape_list : public shape
{
public:
  void add(std::shared_ptr<shape> s) { objects.push_back(s); }

  bool hit(const ray &r, double t_min, double t_max, HitRecord &rec) const override
  {
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

private:
  std::vector<std::shared_ptr<shape>> objects;
};

#endif