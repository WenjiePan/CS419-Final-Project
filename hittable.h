#ifndef HITTABLE_H
#define HITTABLE_H

#include "aabb.h"
#include "ray.h"
#include "utility.h"

class material;

// Structure for holding hit data when the ray hit an object
// Reference: Ray Tracing in One Weekend
struct hit_record {
    point3 p;
    vec3 normal;
    double t;
    double u;
    double v;
    shared_ptr<material> mat_ptr;
    bool front_face;
    color objectColor;

    // Check if hit front face or back face with direction of ray and normal
    inline void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

// Base class for all the hittable objects
// Reference: Ray Tracing in One Weekend
class hittable {
public:
    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
    virtual bool bounding_box(double time0, double time1, aabb& output_box) const = 0;
};

#endif