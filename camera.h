#pragma once

#include "vec3.h"
#include "raytracer.h"
#include "hittable.h"
#include "material.h"
#include "color.h"
#include "ray.h"

class camera
{
public:
    double  aspect_ratio = 1.0;         // Ratio of image width over height
    int     image_width  = 100;         // Rendered image width
    int     samples_per_pixel = 10;     // Count of reandom samples for each pixel (for anti-aliasing)
    int     max_recursion_depth = 10;   // Maximum number of ray bounces

    double vfov     = 90;               // Vertical field of view
    point3 lookfrom = point3(0, 0, 0);  // Point the camera is looking from
    point3 lookat   = point3(0, 0, -1); // Point the camera is looking at
    vec3   vup      = vec3(0, 1, 0);    // camera-relative "up" direction

    double defocus_angle = 0;   // Variation angle of rays through each pixel
    double focus_dist    = 10;  // Distance from the camera lookfrom point to plan of perfect focus

    double time0, time1 = 0.0;    // Open and close time of shutter

    void render(const hittable& world)
    {
        initialize();

        std::cout << "P3\n" << image_width << ' ' << m_image_height << "\n255\n";

        for (int j = 0; j < m_image_height; j++)
        {
            std::clog << "\rScanlines remaining: " << (m_image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++)
            {
                color pixel_color;
                for (int sample = 0; sample < samples_per_pixel; sample++) 
                {
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_recursion_depth, world);
                } 
                write_color(std::cout, m_pixel_sample_scale * pixel_color);
            }
        }

        std::clog << "\rDone.                 \n";
    }


private:
    int     m_image_height;         // Renedered image height
    double  m_pixel_sample_scale;   // Color scale factor for a sum of pixels
    point3  m_center;               // camera center
    point3  m_pixel00_loc;          // Location of pixel 0, 0

    // image pixel size vectors
    vec3    m_pixel_delta_u;        // Offset to pixel to the right
    vec3    m_pixel_delta_v;        // Offset to pixel below
    
    // Camera frame basis vectors
    vec3    m_camera_u;
    vec3    m_camera_v;
    vec3    m_camera_w;

    // Defocus disk horizontal and vertical radii
    vec3    m_defocus_disk_u;
    vec3    m_defocus_disk_v;


    void initialize() 
    {
        m_image_height = int(image_width / aspect_ratio);
        m_image_height = (m_image_height < 1) ? 1 : m_image_height;
        
        m_center = lookfrom;

        m_pixel_sample_scale = 1.0 / samples_per_pixel;

        // viewport dimensions
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta / 2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width) / m_image_height);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        m_camera_w = unit_vector(lookfrom - lookat);
        m_camera_u = unit_vector(cross(vup, m_camera_w));
        m_camera_v = cross(m_camera_w, m_camera_u);

        // Viewport edge vectors
        auto viewport_u = viewport_width * m_camera_u;
        auto viewport_v = viewport_height * -m_camera_v;

        // viewport delta vectors
        m_pixel_delta_u = viewport_u / image_width;
        m_pixel_delta_v = viewport_v / m_image_height;

        // Calculate the camera defocus disk basis vectors
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        m_defocus_disk_u = m_camera_u * defocus_radius;
        m_defocus_disk_v = m_camera_v * defocus_radius;

        // Upper left pixel position
        auto viewport_upper_left = m_center - (focus_dist * m_camera_w) - viewport_u / 2 - viewport_v / 2;
        m_pixel00_loc = viewport_upper_left + m_pixel_delta_u / 2 + m_pixel_delta_v / 2;
    }

    ray get_ray(int i, int j) const
    {
        // Construct a camera ray originating from the defocus disk and directed at a randomly
        // sampled point around the pixel location i, j.
        
        auto offset = sample_square();
        auto pixel_sample = m_pixel00_loc 
                        + m_pixel_delta_u * (i + offset.x()) 
                        + m_pixel_delta_v * (j + offset.y());
        
        point3 ray_origin = (defocus_angle <= 0) ? m_center : defocus_disk_sample();
        point3 ray_direction = pixel_sample - ray_origin;

        // In order to facilitate motion blur, we introduce a shuttertime and each ray 
        // is sampled at a random point in time between the open and close interval
        double time = time0 + random_double() * (time1 - time0);
        
        return ray(ray_origin, ray_direction, time);
    }

    // Returns the vector to a random point in the [-.5, .5]-[-.5, .5] unit square
    vec3 sample_square() const
    {
        return vec3(random_double() - 0.5, random_double() - 0.5, 0.0);
    }

    color ray_color(const ray& r, int depth, const hittable& world) const 
    {
        if (depth <= 0) {
            return color(0,0,0);
        }

        hit_record rec;

        // object color
        if (world.hit(r, interval(0.001, infinity), rec)) {            
            ray scattered;
            color attenuation;
            if (rec.mat_p->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, depth-1, world);
            return color(0,0,0);
        }

        // Background color
        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
    }

    point3 defocus_disk_sample() const 
    {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return m_center + (p[0] * m_defocus_disk_u) + (p[1] * m_defocus_disk_v);
    }
};
