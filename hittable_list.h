#pragma once

#include "aabb.h"
#include "hittable.h"

#include <vector>


class hittable_list : public hittable
{
public:
    std::vector<shared_ptr<hittable>> objects;

    hittable_list() {}
    hittable_list(shared_ptr<hittable> object_p) { add(object_p); }

    void clear() { objects.clear(); }
    void add(shared_ptr<hittable> object_p) 
    {
        objects.push_back(object_p);
        m_bbox = axis_aligned_bounding_box(m_bbox, object_p->bounding_box());
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override
    {
        hit_record temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;

        for (const auto& object_p : objects)
        {
            if (object_p->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }
        return hit_anything;
    }

    axis_aligned_bounding_box bounding_box() const override { return m_bbox; }

private:
    axis_aligned_bounding_box m_bbox;
};