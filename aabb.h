#pragma once

#include "interval.h"
#include "vec3.h"
#include "ray.h"

class axis_aligned_bounding_box 
{
public:
    interval x, y, z;

    axis_aligned_bounding_box() {}
    
    axis_aligned_bounding_box(const interval& x, const interval& y, const interval& z)
        : x(x), y(y), z(z)
    {
        pad_to_minimums();
    }
    
    axis_aligned_bounding_box(const point3& a, const point3& b)
    {
        x = (a[0] <= b[0]) ? interval(a[0], b[0]) : interval(b[0], a[0]);
        y = (a[1] <= b[1]) ? interval(a[1], b[1]) : interval(b[1], a[1]);
        z = (a[2] <= b[2]) ? interval(a[2], b[2]) : interval(b[2], a[2]);

        pad_to_minimums();
    }

    axis_aligned_bounding_box(const axis_aligned_bounding_box& box_a, const axis_aligned_bounding_box& box_b) 
    {
        x = interval(box_a.x, box_b.x);
        y = interval(box_a.y, box_b.y);
        z = interval(box_a.z, box_b.z);
    }

    const interval& axis_interval(int n) const {
        if (n==1) return y;
        if (n==2) return z;
        return x;
    }

    bool hit(const ray& r, interval ray_t) const 
    {
        const point3&   ray_origin = r.origin();
        const vec3&     ray_direction = r.direction();

        for (int axis = 0; axis < 3; axis++) {
            const interval& ax = axis_interval(axis);
            const double adinv = 1.0 / ray_direction[axis];

            auto t0 = (ax.min - ray_origin[axis]) * adinv;
            auto t1 = (ax.max - ray_origin[axis]) * adinv;

            if (t0 < t1) {
                if (t0 > ray_t.min) ray_t.min = t0;
                if (t1 < ray_t.max) ray_t.max = t1;
            } else {
                if (t1 > ray_t.min) ray_t.min = t1;
                if (t0 < ray_t.max) ray_t.max = t0;
            }

            if (ray_t.max <= ray_t.min)
                return false;
        }
        return true;
    }

    int longest_axis() const
    {
        if (x.size() > y.size())
            return x.size() > z.size() ? 0 : 2;
        else
            return y.size() > z.size() ? 1 : 2;
    }

    static const axis_aligned_bounding_box empty, universe;

private:

    void pad_to_minimums()
    {
        double delta = 0.0001;
        if (x.size() < delta) x = x.expand(delta);
        if (y.size() < delta) y = y.expand(delta);
        if (z.size() < delta) z = z.expand(delta);
    }
};

const axis_aligned_bounding_box axis_aligned_bounding_box::empty    = axis_aligned_bounding_box(interval::empty,    interval::empty,    interval::empty);
const axis_aligned_bounding_box axis_aligned_bounding_box::universe = axis_aligned_bounding_box(interval::universe, interval::universe, interval::universe);

axis_aligned_bounding_box operator+(const axis_aligned_bounding_box& bbox, const vec3& offset)
{
    return axis_aligned_bounding_box(bbox.x + offset.x(), bbox.y + offset.y(), bbox.z + offset.z());
}

axis_aligned_bounding_box operator+(const vec3& offset, const axis_aligned_bounding_box& bbox)
{
    return bbox + offset;
}