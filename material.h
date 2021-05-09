#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "texture.h"
#include "utility.h"

#include <string>

struct hit_record;

// Class for material abstract class
// Reference: Ray Tracing: The Next Week
class material {
    public:
        virtual bool scatter(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const = 0;
        virtual color getColor() const = 0;
        virtual color emitted(double u, double v, const point3& p) const {
            return color(0, 0, 0);
        }
        virtual std::string getMatName() const = 0;
};

// Lambertian material, randomly reflects ray around surface normals
class diffuse : public material {
public:
    diffuse(const color& a) : albedo(make_shared<solid_color>(a)) {}
    diffuse(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override {
        auto scatter_direction = unit_vector(rec.normal + random_unit_vector());

        // Catch degenerate scatter direction
        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }
    virtual color getColor() const override {
        return albedo->value(0, 0, vec3());
    }
    virtual color emitted(double u, double v, const point3& p) const override {
        return 0.05 * albedo->value(u, v, p);
    }
    virtual std::string getMatName() const override {
        return "diffuse";
    }

    public:
        shared_ptr<texture> albedo;
};

// Metal meterial, reflects ray perfectly to represent a mirror feature
class specular : public material {
    public:
        specular(const color& a) : albedo(make_shared<solid_color>(a)) {}
        specular(shared_ptr<texture> a) : albedo(a) {}

        virtual bool scatter(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override {
            vec3 reflected = vec3_reflect(unit_vector(r_in.direction()), rec.normal);
            scattered = ray(rec.p, reflected);
            attenuation = albedo->value(rec.u, rec.v, rec.p);
            return (dot(scattered.direction(), rec.normal) > 0);
        }
        virtual color getColor() const override {
            return albedo->value(0, 0, vec3());
        }
        virtual std::string getMatName() const override {
            return "specular";
        }

    public:
        shared_ptr<texture> albedo;
};

// Dielectric material, refract and reflect rays to allow transparency
class refract : public material {
    public:
        refract(double index_of_refraction) : ir(index_of_refraction) {}

        virtual bool scatter(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override {
            attenuation = color(0.99, 0.99, 0.99);
            double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

            vec3 unit_direction = unit_vector(r_in.direction());
            double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
            double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

            bool cannot_refract = refraction_ratio * sin_theta > 1.0;
            vec3 direction;

            if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
                direction = vec3_reflect(unit_direction, rec.normal);
            else
                direction = vec3_refract(unit_direction, rec.normal, refraction_ratio);

            scattered = ray(rec.p, direction);
            return true;
        }
        virtual color getColor() const override {
            return color(0.99, 0.99, 0.99);
        }
        virtual std::string getMatName() const override {
            return "refract";
        }

    public:
        double ir; // Index of Refraction

    private:
        static double reflectance(double cosine, double ref_idx) {
            // Use Schlick's approximation for reflectance.
            auto r0 = (1 - ref_idx) / (1 + ref_idx);
            r0 = r0 * r0;
            return r0 + (1 - r0) * pow((1 - cosine), 5);
        }
};

// Diffuse light material, represent area light
class diffuse_light : public material {
    public:
        diffuse_light(shared_ptr<texture> a) : emit(a) {}
        diffuse_light(color c) : emit(make_shared<solid_color>(c)) {}

        virtual bool scatter(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override {
            return false;
        }
        virtual color emitted(double u, double v, const point3& p) const override {
            return emit->value(u, v, p);
        }
        virtual color getColor() const override {
            return emit->value(0, 0, vec3());
        }
        virtual std::string getMatName() const override {
            return "light";
        }

    public:
        shared_ptr<texture> emit;
};

#endif
