#include "render.h"

#include <iostream>
#include <functional>
#include <fstream>

int main() {
    const auto aspect_ratio = 1.0 / 1.0;
    const int image_width = 128;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 100;
    const int max_depth = 50;
    color background(0,0,0);

    auto world = final_scene();

    point3 lookfrom = point3(478, 278, -600);
    point3 lookat = point3(278, 278, 0);
    vec3 vup(0,1,0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.0;
    auto vfov = 40.0;

    int image_num = 10;
    int thread_num = 2;

    /* const auto aspect_ratio = 2.0 / 1.0;
    const int image_width = 128;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 10;
    const int max_depth = 2;
    color background(0.70, 0.80, 1.00);

    auto world = random_scene();

    point3 lookfrom(13,2,3);
    point3 lookat(0,0,0);
    vec3 vup(0,1,0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.1;
    auto vfov = 20.0;

    int image_num = 30;
    int thread_num = 2; */

    render_multi_nothread(
        image_num, 
        lookfrom, 
        lookat, 
        vup, 
        vfov, 
        aspect_ratio, 
        aperture, 
        dist_to_focus,
        image_width, 
        image_height, 
        samples_per_pixel, 
        world, 
        max_depth, 
        background
    );
    /* render_multi_thread(
        image_num,
        thread_num, 
        lookfrom, 
        lookat, 
        vup, 
        vfov, 
        aspect_ratio, 
        aperture, 
        dist_to_focus, 
        cam, 
        image_width, 
        image_height, 
        samples_per_pixel, 
        world, 
        max_depth, 
        background
    ); */
    return 0;
}