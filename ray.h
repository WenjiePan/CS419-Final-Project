#ifndef RAY_H
#define RAY_H

#include "vec3.h"

// Class for ray initialization
// Reference: Ray Tracing in One Weekend
class ray {
	public:
		point3 orig;
		vec3 dir;
		double tm;

	public:
		ray() {}
		ray(const point3& origin, const vec3& direction, double time = 0.0)
			: orig(origin), dir(direction), tm(time)
		{}

		point3 origin() const { return orig; }
		vec3 direction() const { return dir; }
		double time() const { return tm; }

		// Return the point of ray hit given t
		point3 at(double t) const {
			return orig + t * dir;
		}
};

#endif