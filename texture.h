#pragma once

#include "perlin.h"
#include "rtw_stb_image.h"
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

class image_texture : public texture
{
public:
    image_texture(const char* filename) : image(filename) {}

    color value(double u, double v, const point3& p) const override
    {

        // If we have no texture data, return a solid cyan.
        if (image.height() <= 0) return color(0,1,1);

        // Clamp input texture coordinates to [0,1] x [1,0]
        u = interval(0, 1).clamp(u);
        v = 1.0 - interval(0, 1).clamp(v); // v is flipped to image coordinates

        auto i = int(u * image.width());
        auto j = int(v * image.height());

        auto pixel = image.pixel_data(i, j);

        auto color_scale = 1.0 / 255.0;
        return color(
            color_scale * pixel[0],
            color_scale * pixel[1],
            color_scale * pixel[2]
        );
    }

private:
    rtw_image image;
};

class noise_texture : public texture
{
public:
    noise_texture(double scale) : scale(scale) {}

    color value(double u, double v, const point3& p) const override
    {
        return color(.5, .5, .5) * (1 + std::sin(scale * p.z() + 10 * noise.turb(p, 7)));
    }
private:
    perlin noise;
    double scale;
};