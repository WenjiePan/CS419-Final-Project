#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "hittable.h"
#include "vec3.h"

// Class for triangle hittable object
class triangle : public hittable {
	public:
		point3 p0;
		point3 p1;
		point3 p2;
		color objectColor;
		vec3 normal_v0;
		vec3 normal_v1;
		vec3 normal_v2;

	public:
		triangle() {}
		triangle(point3 v0, point3 v1, point3 v2, color c) : p0(v0), p1(v1), p2(v2), objectColor(c) {
			vec3 e1 = p1 - p0;
			vec3 e2 = p2 - p0;
			vec3 normal = cross(e1, e2);
			normal_v0 = normal;
			normal_v1 = normal;
			normal_v2 = normal;
		}

		vec3 getFaceNormal() const;

		virtual bool hit(
			const ray& r, double t_min, double t_max, hit_record& rec) const override;
		virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;
};

vec3 triangle::getFaceNormal() const {
	vec3 e1 = p1 - p0;
	vec3 e2 = p2 - p0;
	return cross(e1, e2);
}

// Check if ray hit the triangle object
bool triangle::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
	// Algorithm reference: CS 419 Lecture: Ray-Triangle Intersection
	vec3 e1 = p1 - p0;
	vec3 e2 = p2 - p0;
	vec3 normal = cross(e1, e2);
	vec3 qv = cross(r.direction(), e2);
	double a = dot(e1, qv);
	if (a > -epsilon && a < epsilon) return false;
	double f = 1 / a;
	vec3 s = r.origin() - p0;
	double u = f * dot(s, qv);
	if (u < 0.0) return false;
	vec3 rv = cross(s, e1);
	double v = f * dot(r.direction(), rv);
	if (v < 0.0 || u + v > 1.0) return false;
	auto ray_t = f * dot(e2, rv);
	if (ray_t < t_min || t_max < ray_t) return false;

	rec.t = ray_t;
	rec.p = r.at(rec.t);

	// Compute barycentric coodinates to compute face normals with per-vertex normals
	// Algorithm reference: CS 419 Lecture: Ray-Triangle Intersection
	vec3 bary_e0 = p1 - p2;
	vec3 bary_e1 = p2 - p0;
	vec3 bary_e2 = p0 - p1;
	vec3 d0 = rec.p - p0;
	vec3 d1 = rec.p - p1;
	vec3 d2 = rec.p - p2;
	vec3 bary_n = unit_vector(cross(bary_e1, bary_e0));
	double bary0 = dot(cross(bary_e0, d1), bary_n) / dot(cross(bary_e1, bary_e0), bary_n);
	double bary1 = dot(cross(bary_e1, d2), bary_n) / dot(cross(bary_e1, bary_e0), bary_n);
	double bary2 = dot(cross(bary_e2, d0), bary_n) / dot(cross(bary_e1, bary_e0), bary_n);

	vec3 outward_normal = bary0 * normal_v0 + bary1 * normal_v1 + bary2 * normal_v2;
	rec.set_face_normal(r, outward_normal);
	return true;
}

// Return bounding box for triangle object
bool triangle::bounding_box(double time0, double time1, aabb& output_box) const {
	double min_bound[3] = { 0, 0, 0 };
	double max_bound[3] = { 0, 0, 0 };

	// Iterate through all axis to find smallest point and largest point
	for (int i = 0; i < 3; i++) {
		double p0_i = p0[i];
		double p1_i = p1[i];
		double p2_i = p2[i];
		min_bound[i] = fmin(p0_i, fmin(p1_i, p2_i));
		max_bound[i] = fmax(p0_i, fmax(p1_i, p2_i));
	}
	output_box = aabb(
		point3(min_bound[0], min_bound[1], min_bound[2]),
		point3(max_bound[0], max_bound[1], max_bound[2]));
	return true;
}

#endif // !TRIANGLE_H
