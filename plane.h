#ifndef PLANE_H
#define PLANE_H

#include "hittable.h"
#include "vec3.h"

// Class for plane hittable object
class plane : public hittable {
	public:
		point3 point;
		vec3 normal;
		shared_ptr<material> mat_ptr;
	
	public:
		plane() {}
		plane(point3 a, vec3 n, shared_ptr<material> m) : point(a), normal(n), mat_ptr(m) {};

		virtual bool hit(
			const ray& r, double t_min, double t_max, hit_record& rec) const override;
		virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;
};

// Check if ray hit the plane
bool plane::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
	vec3 a_o = point - r.origin();
	auto numerator = dot(a_o, normal);
	auto denominator = dot(r.direction(), normal);

	// No hit if ray if in parallel with plane
	if (denominator == 0) return false;
	auto ray_t = numerator / denominator;
	if (ray_t < t_min || t_max < ray_t) return false;
	
	// Update hit record
	rec.t = ray_t;
	rec.p = r.at(rec.t);
	vec3 outward_normal = normal;
	rec.set_face_normal(r, outward_normal);
	rec.u = 0;
	rec.v = 0;
	rec.mat_ptr = mat_ptr;

	return true;
}

// Return boudning box of place
bool plane::bounding_box(double time0, double time1, aabb& output_box) const {
	// Plane does not have bounding box since it is infinite
	aabb* output_box_addr = &output_box;
	output_box_addr = NULL;
	return false;
}

#endif