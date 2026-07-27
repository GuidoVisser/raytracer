#pragma once

#include "aabb.h"
#include "hittable.h"
#include "hittable_list.h"

#include <algorithm>

class bvh_node : public hittable {
public:
    bvh_node(hittable_list hlist) : bvh_node(hlist.objects, 0, hlist.objects.size()) { }

    bvh_node(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end) 
    {
        // use longest axis for maximum search optimization
        m_bbox = axis_aligned_bounding_box::empty;
        for (size_t object_index=start; object_index < end; object_index++) {
            m_bbox = axis_aligned_bounding_box(m_bbox, objects[object_index]->bounding_box());
        }
        int axis = m_bbox.longest_axis();

        auto comparator = (axis == 0) ? box_x_compare
                        : (axis == 1) ? box_y_compare
                                      : box_z_compare;

        size_t object_span = end - start;

        if (object_span == 1) {
            m_left = m_right = objects[start];
        } else if (object_span == 2) {
            m_left = objects[start];
            m_right = objects[start+1];
        } else {
            std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator);

            auto mid = start + object_span / 2;
            m_left = make_shared<bvh_node>(objects, start, mid);
            m_right = make_shared<bvh_node>(objects, mid, end);
        }

        m_bbox = axis_aligned_bounding_box(m_left->bounding_box(), m_right->bounding_box());
    }

    bool hit (const ray& r, interval ray_t, hit_record& rec) const override
    {
        if (!m_bbox.hit(r, ray_t))
            return false;
        
        bool hit_left   = m_left->hit(r, ray_t, rec);
        
        // This doesn't really make sense to me. We are only evaluating if right got hit if left is not in the way, but 
        // at this point left can still be a sparse bounding volume. Revisit when through all tutorials.
        bool hit_right  = m_right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

        return hit_left || hit_right;
    }

    axis_aligned_bounding_box bounding_box() const override { return m_bbox; }


private:
    shared_ptr<hittable> m_left;
    shared_ptr<hittable> m_right;
    axis_aligned_bounding_box m_bbox;

    static bool box_compare(
        const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis_index
    ) {
        auto a_axis_interval = a->bounding_box().axis_interval(axis_index);
        auto b_axis_interval = b->bounding_box().axis_interval(axis_index);
        return a_axis_interval.min < b_axis_interval.min;
    }

    static bool box_x_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b)
    {
        return box_compare(a, b, 0);
    }

    static bool box_y_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b)
    {
        return box_compare(a, b, 1);
    }

    static bool box_z_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b)
    {
        return box_compare(a, b, 2);
    }
};