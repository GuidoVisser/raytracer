#pragma once

#include "vec3.h"

class ray {
public:
    ray() {}
    ray(const point3& origin, const point3& direction, double time = 0.0) : m_orig(origin), m_dir(direction), m_time(time) {}

    const point3& origin() const { return m_orig; }
    const point3& direction() const { return m_dir; }
    double time() const {return m_time; }

    point3 at(double t) const 
    {
        return m_orig + t * m_dir;
    }

private:
    point3 m_orig;
    point3 m_dir;
    double m_time;
};