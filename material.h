#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "texture.h"
#include "utility.h"

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
};

// Default material that mainly interacts with phong shading
class default_mat : public material {
    public:
        default_mat(const color& a) : albedo(make_shared<solid_color>(a)) {}
        default_mat(shared_ptr<texture> a) : albedo(a) {}

        virtual bool scatter(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override {
            return false;
        }
        virtual color getColor() const override {
            return albedo->value(0, 0, vec3());
        }

    public:
        shared_ptr<texture> albedo;
};

// Lambertian material, randomly reflects ray around surface normals
class lambertian : public material {
public:
    lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {}
    lambertian(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
    ) const override {
        auto scatter_direction = unit_vector(1.1 * rec.normal + random_unit_vector());

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

    public:
        shared_ptr<texture> albedo;
};

// Metal meterial, reflects ray perfectly to represent a mirror feature
class metal : public material {
    public:
        metal(const color& a) : albedo(make_shared<solid_color>(a)) {}
        metal(shared_ptr<texture> a) : albedo(a) {}

        virtual bool scatter(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override {
            vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
            scattered = ray(rec.p, reflected);
            attenuation = albedo->value(rec.u, rec.v, rec.p);
            return (dot(scattered.direction(), rec.normal) > 0);
        }
        virtual color getColor() const override {
            return albedo->value(0, 0, vec3());
        }

    public:
        shared_ptr<texture> albedo;
};

// Dielectric material, refract and reflect rays to allow transparency
class dielectric : public material {
    public:
        dielectric(double index_of_refraction) : ir(index_of_refraction) {}

        virtual bool scatter(
            const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
        ) const override {
            attenuation = color(1.0, 1.0, 1.0);
            double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

            vec3 unit_direction = unit_vector(r_in.direction());
            double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
            double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

            bool cannot_refract = refraction_ratio * sin_theta > 1.0;
            vec3 direction;

            if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
                direction = reflect(unit_direction, rec.normal);
            else
                direction = refract(unit_direction, rec.normal, refraction_ratio);

            scattered = ray(rec.p, direction);
            return true;
        }
        virtual color getColor() const override {
            return color(1, 1, 1);
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

    public:
        shared_ptr<texture> emit;
};

#endif
