#ifndef HITTABLE_LIST
#define HITTABLE_LIST

#include "aabb.h"
#include "hittable.h"

#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;

// Class that holds the vector of hittable objects
// Reference: Ray Tracing in One Weekend
class hittable_list : public hittable {
	public:
		std::vector<shared_ptr<hittable>> objects;

	public:
		hittable_list() {}
		hittable_list(shared_ptr<hittable> object) { add(object); }

		void clear() { objects.clear(); }
		void add(shared_ptr<hittable> object) { objects.push_back(object); }

		virtual bool hit(
			const ray& r, double t_min, double t_max, hit_record& rec) const override;

		virtual bool bounding_box(
			double time0, double time1, aabb& output_box) const override;

		virtual std::string getMatName() const override;

		bool shadow_hit(const ray& r);
};

// Check all the hittable objects in the vector to find the closet hit
bool hittable_list::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
	hit_record temp_rec;
	bool hit_anything = false;
	auto closest_so_far = t_max;

	for (const auto& object : objects) {
		// Find the closest t and update
		if (object->hit(r, t_min, closest_so_far, temp_rec)) {
			hit_anything = true;
			closest_so_far = temp_rec.t;
			rec = temp_rec;
		}
	}

	return hit_anything;
}

// Get the bounding box that surrounds all hittable objects in the vector
bool hittable_list::bounding_box(double time0, double time1, aabb& output_box) const {
	if (objects.empty()) return false;

	aabb temp_box;
	bool first_box = true;

	// Iterate through objects to find the bounding box that surrounds all objects
	for (const auto& object : objects) {
		if (!object->bounding_box(time0, time1, temp_box)) return false;
		output_box = first_box ? temp_box : surrounding_box(output_box, temp_box);
		first_box = false;
	}

	return true;
}

std::string hittable_list::getMatName() const {
	return "";
}

// Check if shadow ray hit any object
bool hittable_list::shadow_hit(const ray& r) {
	hit_record temp_rec;

	for (const auto& object : objects) {
		// Use epsilon to remove possible glitches of hitting itself
		if (object->hit(r, epsilon, 1.0, temp_rec)) {
			return true;
		}
	}

	return false;
}

#endif