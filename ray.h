#ifndef RAY_H
#define RAY_H

#include "vec3.h"

// Class for ray initialization
// Reference: Ray Tracing in One Weekend
class ray {
	public:
		point3 orig;
		vec3 dir;

	public:
		ray() {}
		ray(const point3& origin, const vec3& direction)
			: orig(origin), dir(direction)
		{}

		point3 origin() const { return orig; }
		vec3 direction() const { return dir; }

		void setOrigin(point3 origin) {
			orig = origin;
		}

		void setDirection(vec3 direction) {
			dir = direction;
		}

		// Return the point of ray hit given t
		point3 at(double t) const {
			return orig + t * dir;
		}
};

#endif