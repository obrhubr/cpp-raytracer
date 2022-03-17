#ifndef LENS_H
#define LENS_H

#include "rtweekend.h"
#include "hittable.h"

class lens : public hittable {
    public:
        lens() {}
        lens(point3 cen, point3 dir, double r, double t, shared_ptr<material> m) : center(cen), direction(dir), radius(r), thickness(t), mat_ptr(m) {};

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
        virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;

    public:
        point3 center;
        point3 direction;
        double radius;
        double thickness;
        shared_ptr<material> mat_ptr;

    private:
        static double get_parabola(double radius, double thickness) {
            double x1 = -radius; double y1 = thickness / 100;
            double x2 = 0; double y2 = thickness;
            double x3 = radius; double y3 = thickness / 100;

            double denom = (x1-x2) * (x1-x3) * (x2-x3);
            double A     = (x3 * (y2-y1) + x2 * (y1-y3) + x1 * (y3-y2)) / denom;
            double B     = (x3*x3 * (y1-y2) + x2*x2 * (y3-y1) + x1*x1 * (y2-y3)) / denom;
            double C     = (x2 * x3 * (x2-x3) * y1+x3 * x1 * (x3-x1) * y2+x1 * x2 * (x1-x2) * y3) / denom;

            return A;
        }

        static point3 get_point_on_parabola(double p, point3 intersection_point) {
            // point3 parabola: (var1, var2, var3) -> f(x) = var1 * x^2 + var2 * x + var3
            double ix = intersection_point.x();
            double iy = intersection_point.y();
            double iz = intersection_point.z();
            return point3(p * p * p * ix, p * p * p * iy, p * p * p * iz);
        }

        static point3 get_normal(point3 hit_point, point3 center, double thickness, double radius) {
            point3 intersection_point = hit_point - center;
            // get a parabolic function that describes the curve of the lens
            double parabola = get_parabola(radius, thickness);
            // get the heights of the parabola at the point of intersection between ray and lens
            point3 heights = get_point_on_parabola(parabola, intersection_point);
            // return that height because it corresponds to the normal vector
            return heights;
        };
};

bool lens::bounding_box(double time0, double time1, aabb& output_box) const {
    output_box = aabb(
        center - vec3(radius, radius, radius),
        center + vec3(radius, radius, radius));
    return true;
}

bool lens::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius*radius;

    auto discriminant = half_b*half_b - a*c;
    if (discriminant < 0) { return false; }
    auto sqrtd = sqrt(discriminant);

    auto root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root) {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root) { return false; }
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    vec3 normal = get_normal(rec.p, center, thickness, radius);
    rec.normal = normal;
    rec.set_face_normal(r, normal);
    rec.mat_ptr = mat_ptr;

    return true;
}

#endif