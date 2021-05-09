#include "utility.h"

#include "aarect.h"
#include "bvh.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "jitter.h"
#include "light.h"
#include "material.h"
#include "obj.h"
#include "photon_map.h"
#include "plane.h"
#include "sphere.h"
#include "triangle.h"

#include <chrono>
#include <fstream>
#include <iostream>

int toInt(double x) { return int(pow(1 - exp(-x), 1 / 2.2) * 255 + .5); }

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
    //out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
    //    << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
    //    << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << '\n';
    out << toInt(r) << ' '
        << toInt(g) << ' '
        << toInt(b) << '\n';
}

color ray_color(const ray& r, const hittable_list& world, photon_map& p_map, int max_estimate, int depth) {
    //hit_record rec;

    //if (depth <= 0)
    //    return color(0, 0, 0);

    //if (!world.hit(r, 0.001, infinity, rec))
    //    return background;

    //ray scattered;
    //color attenuation;
    //color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

    //if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
    //    return emitted;

    //return emitted + attenuation * ray_color(scattered, background, world, depth - 1);
    hit_record rec;
    if (depth <= 0 || !world.hit(r, epsilon, infinity, rec)) { return color(0, 0, 0); }

    depth -= 1;
    ray scattered;
    color attenuation;
    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered)) { return rec.mat_ptr->emitted(rec.u, rec.v, rec.p); }
    
    if (rec.mat_ptr->getMatName().compare("diffuse") == 0) {
        vector<photon*> nearest_photons;
        vector<double> dists;
        for (int i = 0; i < max_estimate; i++) {
            nearest_photons.push_back(NULL);
            dists.push_back(infinity);
        }
        p_map.nearest(nearest_photons, dists, rec.p, max_estimate, 0);
        color power = color(0, 0, 0);
        int j;
        for (j = 0; j < max_estimate; j++) {
            photon* curr_p = nearest_photons[j];
            if (curr_p == NULL) {
                break;
            }
            vec3 p_dir = curr_p->dir;
            if (dot(p_dir, rec.normal) < 0.0f) {
                power += curr_p->pow;
            }
        }
        j -= 1;
        power = power * (1.0 / (pi * dists[j] * dists[j]));
        return power;
    }
    else {
        return attenuation * ray_color(scattered, world, p_map, max_estimate, depth - 1);
    }
}

void cornell_box(hittable_list& world) {
    // Create scene
    auto material_left_wall = make_shared<diffuse>(color(0.7, 0.2, 0.2));
    auto material_right_wall = make_shared<specular>(color(0.2, 0.7, 0.2));
    auto material_front_wall = make_shared<diffuse>(color(0, 0, 0));
    auto material_walls = make_shared<diffuse>(color(1, 1, 1));
    auto material_left_ball = make_shared<specular>(color(0.2, 0.2, 0.7));
    auto material_mid_ball = make_shared<diffuse>(color(0.7, 0.7, 0.2));
    auto material_right_ball = make_shared<refract>(1.5);

    world.add(make_shared<sphere>(point3(-(1e5 + 50), 50, 80), 1e5, material_left_wall));
    world.add(make_shared<sphere>(point3(1e5 + 50, 50, 80), 1e5, material_right_wall));
    world.add(make_shared<sphere>(point3(0, 50, 1e5 + 150), 1e5, material_front_wall));
    world.add(make_shared<sphere>(point3(0, 1e5 + 100, 80), 1e5, material_walls));
    world.add(make_shared<sphere>(point3(0, 1e5, 80), 1e5, material_walls));
    world.add(make_shared<sphere>(point3(0, 50, 1e5), 1e5, material_walls));
    world.add(make_shared<sphere>(point3(-25, 16.5, 47), 16.5, material_left_ball));
    world.add(make_shared<sphere>(point3(0, 8.5, 60), 8.5, material_mid_ball));
    world.add(make_shared<sphere>(point3(25, 16.5, 77), 16.5, material_right_ball));
}

void emit_photons(hittable_list& world, vector<photon*>& photons, vector<light*>& lights, int max_photons, int depth) {
    // Only for single light source
    for (int i = 0; i < lights.size(); i++) {
        int n_photons = 0;
        int prev_size = photons.size();
        int curr_size = 0;
        while (n_photons < max_photons) {
            ray photon_ray = lights[i]->randomLightRay();
            photon_tracing(world, photons, photon_ray, lights[i]->getLightColor(), depth);
            curr_size = photons.size();
            if (curr_size - prev_size > 0) {
                n_photons++;
                prev_size = curr_size;
            }
        }

        for (int i = 0; i < photons.size(); i++) {
            photons[i]->pow = photons[i]->pow / n_photons;
        }
    }
}

int main() {
    // Image
    const int image_width = 400;
    const int image_height = 400;
    const int samples_per_pixel = 4;
    const int max_depth = 5;
    const int max_photons = 1200000;
    const int max_estimate = 100;

    // Colors
    color background = color(0, 0, 0);

    // World
    hittable_list world;

    // Light
    vector<light*> lights;
    color lightPow = color(18000, 18000, 18000);
    xz_rect_light* top_light = new xz_rect_light(point3(0, 99.9, 75), lightPow, 20, 10, vec3(0, -1, 0));
    lights.push_back(top_light);

    auto difflight = make_shared<diffuse_light>(lightPow);
    world.add(make_shared<xz_rect>(-10, 10, 70, 80, 99.9, difflight));

    // Create a cornell box
    cornell_box(world);

    // Photon vector
    vector<photon*> raw_photons;

    // Camera
    camera cam(point3(0, 50, 200), point3(0, 48, -1), vec3(0, 1, 0));
    // TODO: Adjust ray from camera to fit front wall (z = 150)

    // Alternative Camera
    camera alt_cam(point3(10, 5, 0), point3(5, 0, -10), vec3(0, 1, 0));

    // Jitter
    jitter jit = jitter(samples_per_pixel);

    // Output File
    std::ofstream output_file("image_cornell_box.ppm");

    // Photon Tracing
    emit_photons(world, raw_photons, lights, max_photons, max_depth);

    // Build photon map with kd-tree
    photon_map global_photon_map = photon_map(raw_photons);

    // Render
    //output_file << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    //std::chrono::steady_clock::time_point render_begin = std::chrono::steady_clock::now();
    //for (int j = image_height - 1; j >= 0; --j) {
    //    std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
    //    for (int i = 0; i < image_width; ++i) {
    //        color pixel_color(0, 0, 0);
    //        for (int s = 0; s < samples_per_pixel; ++s) {
    //            auto u = (i + random_double()) / (image_width - 1);
    //            auto v = (j + random_double()) / (image_height - 1);
    //            ray r = cam.get_ray(u, v);
    //            r.setOrigin(r.origin() + 50.5 * r.direction());
    //            pixel_color += ray_color(r, background, world, max_depth);
    //        }
    //        write_color(output_file, pixel_color, samples_per_pixel);
    //    }
    //}
    //std::chrono::steady_clock::time_point render_end = std::chrono::steady_clock::now();
    //std::cout << "\nRender Time = " << std::chrono::duration_cast<std::chrono::milliseconds>(render_end - render_begin).count() << "[ms]" << std::endl;

    // Render multi-jittered sampling
    output_file << "P3\n" << image_width << ' ' << image_height << "\n255\n";

    std::chrono::steady_clock::time_point render_begin = std::chrono::steady_clock::now();
    for (int j = image_height - 1; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) {
            color pixel_color(0, 0, 0);
            vector<vector<double>> sample_points = jit.get_samples(i, j);
            for (int s = 0; s < samples_per_pixel; ++s) {
                auto u = sample_points[s][0] / (image_width - 1.0);
                auto v = sample_points[s][1] / (image_height - 1.0);
                ray r = cam.get_ray(u, v);
                r.setOrigin(r.origin() + 50.5 * r.direction());
                pixel_color += ray_color(r, world, global_photon_map, max_estimate, max_depth);
            }
            write_color(output_file, pixel_color, samples_per_pixel);
        }
    }
    std::chrono::steady_clock::time_point render_end = std::chrono::steady_clock::now();
    std::cout << "\nRender Time = " << std::chrono::duration_cast<std::chrono::milliseconds>(render_end - render_begin).count() << "[ms]" << std::endl;
}