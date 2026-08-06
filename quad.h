#pragma once

#include "hittable.h"

class quad : public hittable
{
public:
    quad(const point3& Q, const vec3& u, const vec3& v, shared_ptr<material> mat)
        : Q(Q), u(u), v(v), mat(mat)
    {
        auto n = cross(u, v);
        normal = unit_vector(n);
        D = dot(normal, Q);
        w = n / dot(n, n); // Following the tutorial here to see where it goes, but not seeing why w != normal

        set_bounding_box();
    }

    virtual void set_bounding_box()
    {
        auto bbox_diagonal1 = axis_aligned_bounding_box(Q, Q + u + v);
        auto bbox_diagonal2 = axis_aligned_bounding_box(Q + u, Q + v);
        bbox = axis_aligned_bounding_box(bbox_diagonal1, bbox_diagonal2);
    }

    axis_aligned_bounding_box bounding_box() const override { return bbox; }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        auto denom = dot(normal, r.direction());

        // No hit if the ray is parellel to the plane
        if (std::fabs(denom) < 1e-8)
            return false;

        // No hit if the intersection point is outside the ray interval
        auto t = (D - dot(normal, r.origin())) / denom;
        if (!ray_t.contains(t))
            return false;

        // Determine if hit point is within the quad.
        auto intersection = r.at(t);
        vec3 planar_hitpt_vector = intersection - Q;
        auto alpha = dot(w, cross(planar_hitpt_vector, v));
        auto beta  = dot(w, cross(u, planar_hitpt_vector));

        if (!is_interior(alpha, beta, rec))
            return false;

        rec.t = t;
        rec.p = intersection;
        rec.mat_p = mat;
        rec.set_face_normal(r, normal);

        return true;
    }

    // Following tutorial to see where it is going, but I don't like this
    // If it doesn't serve a purpose later, remove the side effect and just write u, v on the
    // hit_record in the hit function.
    //
    // `is_interior` is the only function we'd need to adjust to extend quad to other planar primitives
    // That is probably the reason why u, v are set here to allow for different u,v mappings later.
    virtual bool is_interior(double a, double b, hit_record& rec) const
    {
        interval unit_interval = interval(0, 1);
        // Given the hit point in plane coordinates, return false if it is outside the
        // primitive, otherwise set the hit record UV coordinates and return true.

        if (!unit_interval.contains(a) || !unit_interval.contains(b))
            return false;

        rec.u = a;
        rec.v = b;
        return true;
    }

private:
    point3 Q;
    vec3 u, v;
    vec3 w;
    shared_ptr<material> mat;
    axis_aligned_bounding_box bbox;
    vec3 normal;
    double D;
};