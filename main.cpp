#include "raytracer.h"
#include "texture.h"
#include "camera.h"
#include "bvh.h"
#include "aabb.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"


void bouncing_spheres() 
{
    // World
    hittable_list world;

    // Add ground
    auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(checker)));

    // Add random spheres
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material_p;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material_p = make_shared<lambertian>(albedo);
                    world.add(make_shared<moving_sphere>(center, center + vec3(0, 0.5 * random_double(), 0), 0.0, 1.0, 0.2, sphere_material_p));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material_p = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material_p));
                } else {
                    // glass
                    sphere_material_p = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material_p));
                }
            }
        }
    }

    // Bounding volume hierarchy
    world = hittable_list(make_shared<bvh_node>(world));

    auto material1_p = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1_p));

    auto material2_p = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2_p));

    auto material3_p = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3_p));
    
    // Camera
    camera cam;
    cam.aspect_ratio        = 16.0 / 9.0;
    cam.image_width         = 1200;
    cam.samples_per_pixel   = 50;
    cam.max_recursion_depth = 10;
    cam.time0               = 0.0;
    cam.time1               = 1.0;

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    cam.render(world);
}

void checkered_spheres()
{
    hittable_list world;

    auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));

    world.add(make_shared<sphere>(point3(0,-10, 0), 10, make_shared<lambertian>(checker)));
    world.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));   

    // Camera
    camera cam;
    cam.aspect_ratio        = 16.0 / 9.0;
    cam.image_width         = 1200;
    cam.samples_per_pixel   = 50;
    cam.max_recursion_depth = 10;
    cam.time0               = 0.0;
    cam.time1               = 1.0;

    cam.vfov     = 20;
    cam.lookfrom = point3(13,2,3);
    cam.lookat   = point3(0,0,0);
    cam.vup      = vec3(0,1,0);

    cam.defocus_angle = 0.6;

    cam.render(world);
}


int main()
{
    switch (2) {
        case 1: bouncing_spheres();  break;
        case 2: checkered_spheres(); break;
    }
}