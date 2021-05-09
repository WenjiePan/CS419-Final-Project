#ifndef AABB_H
#define AABB_H

#include "utility.h"

// Class for axis-aligned bounding boxes. This class stores the info about bounding box and test if the ray hit the box.
// Reference: Ray Tracing: The Next Week
class aabb {
	public:
		point3 minimum;
		point3 maximum;
        point3 centroid;

	public:
		aabb() {
            minimum = point3(infinity, infinity, infinity);
            maximum = point3(-infinity, -infinity, -infinity);
        }
		aabb(const point3& a, const point3& b)
			: minimum(a), maximum(b) 
		{
            centroid = (a + b) / 2;
        }

		point3 min() const { return minimum; }
		point3 max() const { return maximum; }
        point3 cen() const { return centroid; }
        bool hit(const ray& r, double t_min, double t_max) const;

        void fit(const point3& p) {
            minimum.e[0] = std::min(minimum.x(), p.x());
            minimum.e[1] = std::min(minimum.y(), p.y());
            minimum.e[2] = std::min(minimum.z(), p.z());
            maximum.e[0] = std::max(maximum.x(), p.x());
            maximum.e[1] = std::max(maximum.y(), p.y());
            maximum.e[2] = std::max(maximum.z(), p.z());
        }
        int sepAxis() const {
            vec3 sepDiff = maximum - minimum;
            double maxDiff = std::max({ sepDiff.x(), sepDiff.y(), sepDiff.z() });
            if (maxDiff == sepDiff.x()) { return 0; }
            else if (maxDiff == sepDiff.y()) { return 1; }
            else if (maxDiff == sepDiff.z()) { return 2; }
        }
};

// Check if the ray hit the bounding box by computing t_next in 3 axis
inline bool aabb::hit(const ray& r, double t_min, double t_max) const {
    for (int a = 0; a < 3; a++) {
        auto invD = 1.0f / r.direction()[a];
        auto t0 = (min()[a] - r.origin()[a]) * invD;
        auto t1 = (max()[a] - r.origin()[a]) * invD;
        if (invD < 0.0f)
            std::swap(t0, t1);
        t_min = t0 > t_min ? t0 : t_min;
        t_max = t1 < t_max ? t1 : t_max;
        if (t_max <= t_min)
            return false;
    }
    return true;
}

// Return the smallest aabb that contains box0 and box1
aabb surrounding_box(aabb box0, aabb box1) {
    // Get the smallest point in box0 and box1
    point3 small(fmin(box0.min().x(), box1.min().x()),
        fmin(box0.min().y(), box1.min().y()),
        fmin(box0.min().z(), box1.min().z()));

    // Get the largest point in box0 and box1
    point3 big(fmax(box0.max().x(), box1.max().x()),
        fmax(box0.max().y(), box1.max().y()),
        fmax(box0.max().z(), box1.max().z()));

    return aabb(small, big);
}

#endif