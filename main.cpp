#include "utility.h"

#include "aarect.h"
#include "bvh.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "jitter.h"
#include "light.h"
#include "material.h"
#include "moving_sphere.h"
#include "obj.h"
#include "plane.h"
#include "sphere.h"
#include "triangle.h"

#include <chrono>
#include <fstream>
#include <iostream>

void write_color(std::ofstream& out, color pixel_color, int samples_per_pixel) {
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // Divide the color by the number of samples.
    auto scale = 1.0 / samples_per_pixel;
    r *= scale;
    g *= scale;
    b *= scale;

    // Write the translated [0,255] value of each color component.
    out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
        << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
}

color ray_color(const ray& r, const hittable_list& world, int depth) {
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0, 0, 0);

    if (world.hit(r, 0.001, infinity, rec)) {
        ray scattered;
        color attenuation;
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth - 1);
        return color(0, 0, 0);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

int main() {
    // Image
    const int image_width = 400;
    const int image_height = 400;
    const int samples_per_pixel = 100;
    const int max_depth = 50;

    // Colors
    color background = color(0, 0, 0);
    vector<color> color_pool;
    color_pool.push_back(color(1, 0, 0));
    color_pool.push_back(color(0, 1, 0));
    color_pool.push_back(color(0, 0, 1));
    color_pool.push_back(color(1, 1, 0));
    color_pool.push_back(color(1, 0, 1));
    color_pool.push_back(color(0, 1, 1));

    // Obj
    //obj o1("cow.obj");
    //o1.update_vertex_normals();

    // Boxes
    hittable_list box1;
    point3 pos;
    double radii;
    for (int i = 0; i < 1000; i++) {
        pos = point3(random_double(-2, 2), random_double(-2, 2), random_double(-2.1, -4));
        radii = 0.04;
        //box1.add(make_shared<sphere>(pos, radii, color_pool[random_int(0, color_pool.size() - 1)]));
    }

    // World
    hittable_list world;
    auto material_plane = make_shared<lambertian>(color(0.7, 0.7, 0.7));
    auto material_sphere1 = make_shared<lambertian>(color(0.4, 1, 1));
    auto material_sphere2 = make_shared<lambertian>(color(0.6, 0.2, 0.1));

    world.add(make_shared<plane>(point3(0.0, -2.5, -1.0), vec3(0.0, 1.0, 0.0), material_plane));
    point3 center1_1 = point3(0.0, 0.2, -1.0);
    point3 center1_2 = center1_1 + vec3(0, 0.25, 0);
    point3 center2_1 = point3(0.8, -0.3, -1.4);
    point3 center2_2 = center2_1 + vec3(0, 0.4, 0);
    world.add(make_shared<moving_sphere>(center1_1, center1_2, 0.0, 1.0, 0.5, material_sphere1));
    world.add(make_shared<moving_sphere>(center2_1, center2_2, 0.0, 1.0, 0.4, material_sphere2));

    // Camera
    camera cam(point3(0, 0, 1), point3(0, 0, -1), vec3(0, 1, 0), 0.0, 1.0);

    // Alternative Camera
    camera alt_cam(point3(10, 5, 0), point3(5, 0, -10), vec3(0, 1, 0));

    // Light
    light spotLight = light(point3(0, 10, 5), color(0.9, 0.9, 0.9));

    // Jitter
    jitter jit = jitter(samples_per_pixel);

    // Output File
    std::ofstream output_file("image_test.ppm");

    // Render perspective
    output_file << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    std::chrono::steady_clock::time_point render_begin = std::chrono::steady_clock::now();
    for (int j = image_height - 1; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = (i + random_double()) / (image_width - 1);
                auto v = (j + random_double()) / (image_height - 1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, world, max_depth);
            }
            write_color(output_file, pixel_color, samples_per_pixel);
        }
    }
    std::chrono::steady_clock::time_point render_end = std::chrono::steady_clock::now();
    std::cout << "\nRender Time = " << std::chrono::duration_cast<std::chrono::milliseconds>(render_end - render_begin).count() << "[ms]" << std::endl;

    // Render alternative perspective
    //std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    //for (int j = image_height - 1; j >= 0; --j) {
    //    std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
    //    for (int i = 0; i < image_width; ++i) {
    //        auto u = double(i) / (image_width - 1);
    //        auto v = double(j) / (image_height - 1);
    //        ray r = alt_cam.get_ray(u, v);
    //        color pixel_color = ray_color(r, world, spotLight);
    //        write_color(std::cout, pixel_color, 1);
    //    }
    //}

    // Render multi-jittered sampling
    //output_file << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    //std::chrono::steady_clock::time_point render_begin = std::chrono::steady_clock::now();
    //for (int j = image_height - 1; j >= 0; --j) {
    //    std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
    //    for (int i = 0; i < image_width; ++i) {
    //        color pixel_color(0, 0, 0);
    //        vector<vector<double>> sample_points = jit.get_samples(i, j);
    //        for (int s = 0; s < samples_per_pixel; ++s) {
    //            auto u = sample_points[s][0] / (image_width - 1.0);
    //            auto v = sample_points[s][1] / (image_height - 1.0);
    //            ray r = alt_cam.get_ray(u, v);
    //            pixel_color += ray_color(r, background, world, max_depth);
    //        }
    //        write_color(output_file, pixel_color, samples_per_pixel);
    //    }
    //}
    //std::chrono::steady_clock::time_point render_end = std::chrono::steady_clock::now();
    //std::cout << "\nRender Time = " << std::chrono::duration_cast<std::chrono::milliseconds>(render_end - render_begin).count() << "[ms]" << std::endl;
}