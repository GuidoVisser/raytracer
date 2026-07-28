#pragma once

#include "vec3.h"
#include "color.h"

class texture 
{
public:
    virtual ~texture() = default;

    virtual color value(double u, double v, const point3& p) const = 0;

};

class solid_color : public texture
{
public:
    solid_color(const color& albedo) : m_albedo(albedo) {}

    solid_color(double red, double green, double blue) : m_albedo(color(red, green, blue)) {}

    color value(double u, double v, const point3& p) const override
    {
        return m_albedo;
    }

private:
    color m_albedo;
};

class checker_texture : public texture
{
public:
    checker_texture(double scale, shared_ptr<texture>(even), shared_ptr<texture>(odd))
        : m_inv_scale(1.0 / scale), m_even(even), m_odd(odd) {}

    checker_texture(double scale, const color& c1, const color& c2) 
        : checker_texture(scale, make_shared<solid_color>(c1), make_shared<solid_color>(c2)) {}

    color value(double u, double v, const point3& p) const override
    {
        auto x_integer = int(std::floor(m_inv_scale * p.x()));
        auto y_integer = int(std::floor(m_inv_scale * p.y()));
        auto z_integer = int(std::floor(m_inv_scale * p.z()));

        bool is_even = (x_integer + y_integer + z_integer) % 2 == 0;

        return is_even ? m_even->value(u, v, p) : m_odd->value(u, v, p);
    }

private:
    double m_inv_scale;
    shared_ptr<texture> m_even;
    shared_ptr<texture> m_odd;
};