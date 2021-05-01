#ifndef CAMERA_H
#define CAMERA_H

#include "utility.h"

// Class for camera. It determine the origin and look direction of camera to control ray direction
class camera {
	private:
        point3 origin;
        point3 lower_left_corner;
        point3 lower_left_corner_ortho;
        vec3 horizontal;
        vec3 vertical;
        vec3 cam_dir;

    public:
        // Construct camera coordinate based on eye point, lookat direction and lookup direction
        camera(point3 lookfrom, point3 lookat, vec3 vup) {
            auto viewport_height = 2.0;
            auto viewport_width = 2.0;
            auto focal_length = 1.0;

            auto w = unit_vector(lookfrom - lookat);
            auto u = unit_vector(cross(vup, w));
            auto v = cross(w, u);

            origin = lookfrom;
            horizontal = viewport_width * u;
            vertical = viewport_height * v;
            lower_left_corner = origin - horizontal / 2 - vertical / 2 - w;
        }

        // Get perspective ray given the point on viewport
        ray get_ray(double s, double t) const {
            return ray(origin, lower_left_corner + s * horizontal + t * vertical - origin);
        }

        // Get orthographic ray given the point on viewport
        ray get_ortho_ray(double s, double t) const {
            return ray(lower_left_corner_ortho + s * horizontal + t * vertical, unit_vector(cam_dir));
        }
};

#endif
