#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"

#include <functional>

class camera {
    public:
        camera(
            point3 lookfrom,
            point3 lookat,
            vec3   vup,
            double vfov, // vertical field-of-view in degrees
            double aspect_ratio,
            double aperture,
            double fd,
            std::function<point3 (int)> f
        ) {
            auto theta = degrees_to_radians(vfov);
            auto h = tan(theta/2);
            viewport_height = 2.0 * h;
            viewport_width = aspect_ratio * viewport_height;

            focus_dist = fd;

            w = unit_vector(lookfrom - lookat);
            u = unit_vector(cross(vup, w));
            v = cross(w, u);

            origin = lookfrom;
            horizontal = focus_dist * viewport_width * u;
            vertical = focus_dist * viewport_height * v;
            lower_left_corner = origin - horizontal/2 - vertical/2 - focus_dist*w;

            lens_radius = aperture / 2;
            func = f;
        }


        ray get_ray(double s, double t, int time) const {
            auto f = point3(0,0,0);//func(time);
            point3 o = origin - f;

            auto hori = focus_dist * viewport_width * u;
            auto vert = focus_dist * viewport_height * v;
            auto llc = origin - hori/2 - vert/2 - focus_dist*w;

            vec3 rd = lens_radius * random_in_unit_disk();
            vec3 offset = u * rd.x() + v * rd.y();

            return ray(
                o + offset,
                (llc-f) + s*(hori-f) + t*(vert-f) - o - offset
            );
        }

    private:
        point3 origin;
        point3 lower_left_corner;
        vec3 horizontal;
        vec3 vertical;
        vec3 u, v, w;
        double lens_radius;
        double focus_dist;
        double viewport_width, viewport_height;
        std::function<point3 (int)> func;
};

#endif