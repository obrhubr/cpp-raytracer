#include <atomic>
#include <cfloat>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <cmath>
#include <fstream>

#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "material.h"
#include "aarect.h"
#include "box.h"
#include "constant_medium.h"
#include "render.h"

//defining global consts

const auto aspect_ratio = 1.0 / 1.0;
const int image_width = 64;
const int image_height = static_cast<int>(image_width / aspect_ratio);
const int samples_per_pixel = 100;
const int max_depth = 100;
color background(1,1,1);

auto world = final_scene();

point3 lookfrom = point3(478, 278, -600);
point3 lookat = point3(278, 278, 0);
vec3 vup(0,1,0);
auto dist_to_focus = 10.0;
auto aperture = 0.0;
auto vfov = 40.0;

camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus);

int image_num = 10;
int thread_num = 6;

constexpr unsigned N = 16;

constexpr unsigned W_CNT = (image_width + N - 1) / N;
constexpr unsigned H_CNT = (image_height + N - 1) / N;

std::atomic<unsigned> done_count;

struct Pixels {
    Pixels(unsigned w, unsigned h)
        : width{w},
                height{h},
                data{new float[width * height * 5]()},       // RGBA + sample count
                pixels{new uint8_t[width * height * 4]()}  // RGBA
    {}

    ~Pixels() {
        delete[] data;
        delete[] pixels;
    }

    uint8_t *get_pixels() {
        // convert accumulated pixels values so we can display them
        for (int i = 0; i < image_height; i++)
        for (int j = 0; j < image_width; j++) {
            const unsigned data_pos = (i * width + j) * 3;

            auto r = data[data_pos + 0];
            auto g = data[data_pos + 1];
            auto b = data[data_pos + 2];
        
            auto scale = 1.0 / samples_per_pixel;
            r = sqrt(scale * r);
            g = sqrt(scale * g);
            b = sqrt(scale * b);

            pixels[data_pos + 0] = static_cast<int>(256 * clamp(r, 0.0, 0.999));
            pixels[data_pos + 1] = static_cast<int>(256 * clamp(g, 0.0, 0.999));
            pixels[data_pos + 2] = static_cast<int>(256 * clamp(b, 0.0, 0.999));
        }

        return pixels;
    }

    inline void accumulate(unsigned x, unsigned y, float r, float g, float b) {
        const unsigned pos = (y * width + x) * 3;
        data[pos + 0] = r;
        data[pos + 1] = g;
        data[pos + 2] = b;
    }

    inline void accumulate(unsigned x, unsigned y, const vec3 &col) {
        accumulate(x, y, col.x(), col.y(), col.z());
    }

    unsigned width;
    unsigned height;
    float *data;  // RGBA + sample count

    private:
        uint8_t *pixels;  // RGBA

} pixels{image_width, image_height};

struct Task {
    Task() : my_id{id++} {}

    Task(int x, int y) : sx{x}, sy{y}, my_id{id++} {}

    void move_in_pattern(int &rx, int &ry) {
        // snake pattern implementation
        static int x = -1, y = H_CNT - 1;
        static int dir = 0;

        x = dir ? x - 1 : x + 1;
        if (x == W_CNT || x == -1) {
            x = y & 1 ? W_CNT - 1 : 0;
            y--;
            dir = !dir;
        }
        rx = x;
        ry = y;
    }

    bool get_next_task() {
        static bool taken[H_CNT][W_CNT] = {};
        static std::mutex m;

        std::lock_guard<std::mutex> guard{m};

        bool found = false;
        int x, y;
        while (!found) {
            move_in_pattern(x, y);
            if (x < 0 || x > W_CNT || y < 0 || y > H_CNT) break;

            if (!taken[y][x]) {
                sx = x * N;
                sy = y * N;
                taken[y][x] = true;
                found = true;
            }
        }
        return found;
    }

    void operator()() {
        bool done = false;
        do {
            if (!get_next_task()) {
                done = true;
                continue;
            }
            
            for (unsigned y = sy; y < sy + N; y++)
            for (unsigned x = sx; x < sx + N; x++) {
                if (x < 0 || y < 0 || x >= image_width || y >= image_height) continue;
                color col = color(0,0,0);
                for (unsigned s = 0; s < samples_per_pixel; s++) {
                    const float u = float(x + random_double()) / float(image_width);
                    const float v = float(y + random_double()) / float(image_height);
                    ray r = cam.get_ray(u, v);
                    col += ray_color(r, background, world, max_depth);
                }
                pixels.accumulate(x, y, col);
            }
        } while (!done);

        done_count++;

        std::cout << "Thread " << my_id << " is done!" << std::endl;
    }

    int sx = -1, sy = -1;
    int my_id;
    static int id;
};

int Task::id = 0;

int main() {
    const unsigned int n_threads = std::thread::hardware_concurrency() / 2;
    std::cout << "Detected " << n_threads << " concurrent threads." << std::endl;
    std::vector<std::thread> threads{n_threads};

    for (auto &t : threads) t = std::thread{Task{}};

    std::cout << "Waiting for all the threads to join." << std::endl;
    for (auto &t : threads) t.join();

    auto image = pixels.get_pixels();

    std::ofstream ofs("./output/block.ppm", std::ios::out | std::ios::binary);
    ofs << "P6\n" << image_width << " " << image_height << "\n255\n";
    for (unsigned i = 0; i < image_width * image_height; ++i) {
        ofs << image[i*3+0] <<
               image[i*3+1] <<
               image[i*3+2];
    }
    ofs.close();

    return 0;
}