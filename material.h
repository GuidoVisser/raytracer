#pragma once

#include "hittable.h"
#include "texture.h"
#include "vec3.h"
#include "color.h"
#include "ray.h"


class material
{
public:
    virtual ~material() = default;

    virtual color emitted(double u, double v, const point3& p) const
    {
        return color(0,0,0);
    }

    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const
    {
        return false;
    }
};

class lambertian : public material
{
public:
    lambertian(const color& albedo) : m_texture(make_shared<solid_color>(albedo)) {}
    lambertian(shared_ptr<texture> texture) : m_texture(texture) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override
    {
        auto scatter_direction = rec.normal + random_unit_vector();
        
        // if random vector == rec.normal we get degenerate scatter vector
        if (scatter_direction.near_zero())
            // scatter_direction = rec.normal;
            return false; // don't scatter, removes artifacts in cornell box.
        
        scattered = ray(rec.p, scatter_direction, r_in.time());
        attenuation = m_texture->value(rec.u, rec.v, rec.p);
        return true;
    }

private:
    shared_ptr<texture> m_texture;
};

class metal : public material
{
public:
    metal(const color& albedo, double fuzz) : m_albedo(albedo), m_fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override
    {
        auto reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + (m_fuzz * random_unit_vector());
        scattered = ray(rec.p, reflected, r_in.time());
        attenuation = m_albedo;
        return (dot(scattered.direction(), rec.normal) > 0);
    }

private:
    color m_albedo;
    double m_fuzz;
};

class uniform_diffuse : public material
{
public:
    uniform_diffuse(const color& albedo) : m_albedo(albedo) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override
    {
        auto scatter_direction = random_on_hemisphere(rec.normal);
        
        // if random vector == rec.normal we get degenerate scatter vector
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;
        
        scattered = ray(rec.p, scatter_direction, r_in.time());
        attenuation = m_albedo;
        return true;
    }

private:
    color m_albedo;
};

class dielectric : public material {
  public:
    dielectric(double refraction_index) : m_refraction_index(refraction_index) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
    const override {
        attenuation = color(1.0, 1.0, 1.0);
        double ri = rec.front_face ? (1.0 / m_refraction_index) : m_refraction_index;

        vec3 unit_direction = unit_vector(r_in.direction());
        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = std::sqrt(1 - cos_theta*cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, ri) > random_double())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, ri);
        

        scattered = ray(rec.p, direction, r_in.time());
        return true;
    }

  private:
    // Refractive index in vacuum or air, or the ratio of the material's refractive index over
    // the refractive index of the enclosing media
    double m_refraction_index;


    static double reflectance(double cosine, double refraction_index)
    {
        // Use Schlick's approximation for reflectance
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0*r0;
        return r0 + (1-r0)*std::pow((1-cosine), 5);

    }
};

class diffuse_light : public material
{
public:
    diffuse_light(shared_ptr<texture> tex) : tex(tex) {}
    diffuse_light(const color& emit) : tex(make_shared<solid_color>(emit)) {}

    color emitted(double u, double v, const point3& p) const override
    {
        return tex->value(u, v, p);
    }

private:
    shared_ptr<texture> tex;
};