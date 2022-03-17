#include "render.h"
#include "camera.h"

#include <iostream>
#include <functional>
#include <fstream>
#include <string.h>

int main() {
    const auto aspect_ratio = 1.0 / 1.0;
    const int image_width = 512;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 50;
    const int max_depth = 10;
    color background(1,1,1);

    int o = 0;
    for(double j = 0.5; j < 2; j += 0.3) {
        for(double i = -2; i < 2; i += 0.5) {
            point3 lookfrom = point3(i, 1, -2);
            point3 lookat = point3(0, 1, -7);
            vec3 vup(0,1,0);
            auto dist_to_focus = (lookfrom - lookat).length();
            auto aperture = 0.0;
            auto vfov = 40.0;

            auto cam = camera(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus);

            auto world = di_test(j);

            render_image(
                cam,
                std::to_string(o),
                image_width,
                image_height,
                samples_per_pixel,
                world,
                max_depth,
                background,
                1
            );

            o++;
        }
    }

    return 0;
}