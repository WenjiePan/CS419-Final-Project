#ifndef LIGHT_H
#define LIGHT_H

#include "hittable.h"
#include "material.h"
#include "ray.h"
#include "vec3.h"

inline vec3 random_in_unit_hemisphere(int axis, int posFlag) {
	double x, y, z;
	if (axis == 0) {
		while (true) {
			x = posFlag ? random_double(0, 1) : random_double(-1, 0);
			y = random_double(-1, 1);
			z = random_double(-1, 1);
			if (x * x + y * y + z * z > 1) continue;
			return unit_vector(vec3(x, y, z));
		}	
	}
	else if (axis == 1) {
		while (true) {
			x = random_double(-1, 1);
			y = posFlag ? random_double(0, 1) : random_double(-1, 0);
			z = random_double(-1, 1);
			if (x * x + y * y + z * z > 1) continue;
			return unit_vector(vec3(x, y, z));
		}
	}
	else if (axis == 2) {
		while (true) {
			x = random_double(-1, 1);
			y = random_double(-1, 1);
			z = posFlag ? random_double(0, 1) : random_double(-1, 0);
			if (x * x + y * y + z * z > 1) continue;
			return unit_vector(vec3(x, y, z));
		}
	}
}


// Class for light source. It intialize parallel point light and compute phong shading
class light {
	public:
		virtual point3 getPos() const = 0;
		virtual color getLightColor() const = 0;
		virtual ray randomLightRay() const = 0;
};

class xz_rect_light : public light {
	public:
		vec3 lightPos;
		color lightColor;
		float x_len;
		float z_len;
		vec3 outward_norm;

	public:
		xz_rect_light() {}
		xz_rect_light(vec3 p, vec3 c, float x, float z, vec3 n) {
			lightPos = p;
			lightColor = c;
			x_len = x;
			z_len = z;
			outward_norm = n;
		}

		point3 getPos() const override { return lightPos; }
		color getLightColor() const override { return lightColor; }

		float getX_Len() { return x_len; }
		float getZ_Len() { return z_len; }

		ray randomLightRay() const override {
			vec3 rdnPos = vec3(random_double(lightPos.x() - x_len / 2, lightPos.x() + x_len / 2),
								lightPos.y(), 
								random_double(lightPos.z() - z_len / 2, lightPos.z() + z_len / 2));
			vec3 rdnDir = random_in_unit_hemisphere(1, outward_norm[1] > 0);
			return ray(rdnPos, rdnDir);
		}
};
#endif