# pragma once

#include "hittable.h"

class sphere : public hittable
{
public:
    sphere(const point3& center, double radius, shared_ptr<material> mat_p) : m_center(center), m_radius(std::fmax(radius, 0)), m_mat_p(mat_p)
    { }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override
    {
        vec3 oc = m_center - r.origin();
        auto a = r.direction().length_squared();
        auto h = dot(r.direction(), oc);
        auto c = oc.length_squared() - m_radius * m_radius;
        auto discriminant = h * h - a * c;

        if (discriminant < 0) {
            return false;
        }

        auto sqrtd = std::sqrt(discriminant);

        auto root = (h - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root)) {
                return false;
            }
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - m_center) / m_radius;
        rec.set_face_normal(r, outward_normal);
        rec.mat_p = m_mat_p;

        return true;
    }

private:
    point3 m_center;
    double m_radius;
    shared_ptr<material> m_mat_p;
};

class moving_sphere : public hittable
{
public:
    moving_sphere(const point3& center0, const point3& center1, double t0, double t1, double radius, shared_ptr<material> mat_p) 
        : m_center0(center0), m_center1(center1), m_t0(t0), m_t1(t1), m_radius(std::fmax(radius, 0)), m_mat_p(mat_p)
    { }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override
    {
        vec3 oc = center(r.time()) - r.origin();
        auto a = r.direction().length_squared();
        auto h = dot(r.direction(), oc);
        auto c = oc.length_squared() - m_radius * m_radius;
        auto discriminant = h * h - a * c;

        if (discriminant < 0) {
            return false;
        }

        auto sqrtd = std::sqrt(discriminant);

        auto root = (h - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root)) {
                return false;
            }
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - center(r.time())) / m_radius;
        rec.set_face_normal(r, outward_normal);
        rec.mat_p = m_mat_p;

        return true;
    }

    point3 center(double time) const
    {
        if (m_t1 - m_t0 == 0) return m_center0;
        return m_center0 + ((time - m_t0) / (m_t1 - m_t0)) * (m_center1 - m_center0);
    }

private:
    point3 m_center0, m_center1;
    double m_t0, m_t1;
    double m_radius;
    shared_ptr<material> m_mat_p;
};