#ifndef RENDER_H
#define RENDER_H

#include "rtweekend.h"

#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "material.h"
#include "aarect.h"
#include "box.h"
#include "constant_medium.h"
#include "render.h"
#include "bvh.h"

#include <iostream>
#include <functional>
#include <fstream>
#include <thread>

color ray_color(const ray& r, const color& background, const hittable& world, int depth) {
    hit_record rec;

    if (depth <= 0) { return color(0,0,0); }

    // If the ray hits nothing, return the background color.
    if (!world.hit(r, 0.001, infinity, rec))
        return background;

    ray scattered;
    color attenuation;
    color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        return emitted;

    return emitted + attenuation * ray_color(scattered, background, world, depth-1);
}

hittable_list random_scene() {
    hittable_list boxes1;
    auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));

    const int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i*w;
            auto z0 = -1000.0 + j*w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_double(1,101);
            auto z1 = z0 + w;

            boxes1.add(make_shared<box>(point3(x0,y0,z0), point3(x1,y1,z1), ground));
        }
    }

    hittable_list objects;

    objects.add(make_shared<bvh_node>(boxes1, 0, 1));

    auto light = make_shared<diffuse_light>(color(7, 7, 7));
    objects.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));

    auto center1 = point3(400, 400, 200);
    auto moving_sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
    objects.add(make_shared<sphere>(center1, 50, moving_sphere_material));

    objects.add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
    objects.add(make_shared<sphere>(
        point3(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)
    ));

    auto boundary = make_shared<sphere>(point3(360,150,145), 70, make_shared<dielectric>(1.5));
    objects.add(boundary);
    objects.add(make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
    boundary = make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
    objects.add(make_shared<constant_medium>(boundary, .0001, color(1,1,1)));

    hittable_list boxes2;
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxes2.add(make_shared<sphere>(point3::random(0,165), 10, white));
    }

    objects.add(make_shared<translate>(
        make_shared<rotate_y>(
            make_shared<bvh_node>(boxes2, 0.0, 1.0), 15),
            vec3(-100,270,395)
        )
    );

    return objects;
}

point3 circle_motion(int i) {
    double theta = i / 5;
    double r = 13;
    return point3(std::cos(theta)*r, 0, std::sin(theta)*r);
}

void render_image(camera cam, std::string num, int image_width, int image_height, int samples_per_pixel, hittable_list world, int max_depth, color background, int time) {
    std::ofstream ppm;
    ppm.open ("output/image" + num + ".ppm");
    ppm << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    for (int j = image_height-1; j >= 0; --j) {
        std::cerr << "\r" + num + ": Scanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width-1);
                auto v = (j + random_double()) / (image_height-1);
                ray r = cam.get_ray(u, v, time);
                pixel_color += ray_color(r, background, world, max_depth);
            }
            write_color(ppm, pixel_color, samples_per_pixel);
        }
    }
    ppm.close();
    std::cerr << "\nDone: " + num + "\n";
}

void render_multi_nothread(int image_num, point3 lookfrom, point3 lookat, point3 vup, double vfov, double aspect_ratio, double aperture, double dist_to_focus, camera cam, int image_width, int image_height, int samples_per_pixel, hittable_list world, int max_depth, color background) {
    for(int i = 0; i < image_num; i++) {
        
        std::string num = std::string(3 - std::to_string(i).length(), '0') + std::to_string(i);
        render_image(cam, num, image_width, image_height, samples_per_pixel, world, max_depth, background, i);
    }
}

void render_multi_thread(int image_num, int thread_num, point3 lookfrom, point3 lookat, point3 vup, double vfov, double aspect_ratio, double aperture, double dist_to_focus, camera cam, int image_width, int image_height, int samples_per_pixel, hittable_list world, int max_depth, color background) {
    for(int i = 0; i < std::ceil(image_num/thread_num); i++) {
        std::vector<std::thread> all_threads;
        for(int j = 0; j < thread_num; j++) {
            std::string num = std::string(3 - std::to_string((j + i*thread_num)).length(), '0') + std::to_string((j + i*thread_num));
            std::thread t(render_image, cam, num, image_width, image_height, samples_per_pixel, world, max_depth, background, i*thread_num+j);
            all_threads.push_back(std::move(t));
        }
        for(int j = 0; j < all_threads.size(); j++) {
            all_threads[j].join();
        };
    }
}

#endif