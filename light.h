#ifndef LIGHT_H
#define LIGHT_H

#include "hittable.h"
#include "material.h"
#include "vec3.h"

// Class for light source. It intialize parallel point light and compute phong shading
class light {
	private:
		point3 lightPos;
		color lightColor;
		const double ambientStrength = 0.2;
		const double specularStrength = 0.5;

	public:
		light() : lightPos(point3(0, 0, 0)), lightColor(color(1.0, 1.0, 1.0)) {}
		light(point3 pos, color col) {
			lightPos = pos;
			lightColor = col;
		}

		// Get color on 1 pixel with phong shading model
		color phong_shading(const hit_record& rec, hittable_list world, const ray& r) {
			color objectColor = rec.mat_ptr->getColor();
			vec3 unit_normal = unit_vector(rec.normal);
			if (!rec.front_face) {
				unit_normal *= -1;
			}
			vec3 unit_lightDir = unit_vector(lightPos - rec.p);
			ray shadow_ray = ray(rec.p, lightPos - rec.p);
			
			// Ambient
			color ambientColor = (ambientStrength * lightColor) * objectColor;

			// Diffuse
			double nl_dot = std::max(dot(unit_normal, unit_lightDir), 0.0);
			color diffuseColor = nl_dot * lightColor * objectColor;

			// Specular
			vec3 unit_view = unit_vector(r.origin() - rec.p);
			vec3 unit_reflectDir = 2 * dot(unit_lightDir, unit_normal) * unit_normal - unit_lightDir;
			double rv_dot = std::max(dot(unit_reflectDir, unit_view), 0.0);
			color specularColor = pow(rv_dot, 32) * specularStrength * objectColor;

			// If object is in shadow, only return ambient light, otherwise return the sum of all lights
			return ambientColor + ((world.shadow_hit(shadow_ray)) ? (vec3(0, 0, 0) + diffuseColor + specularColor) / 3.0 : diffuseColor + specularColor);
		}
};

#endif