#ifndef TEXTURE_H
#define TEXTURE_H

#include "utility.h"
#include "perlin.h"

// Class for texture abstract class
// Reference: Ray Tracing: The Next Week
class texture {
public:
    virtual color value(double u, double v, const point3& p) const = 0;
};

// Solid color texture, always return single color
class solid_color : public texture {
public:
    solid_color() {}
    solid_color(color c) : color_value(c) {}

    solid_color(double red, double green, double blue)
        : solid_color(color(red, green, blue)) {}

    virtual color value(double u, double v, const vec3& p) const override {
        return color_value;
    }

private:
    color color_value;
};

class checker_3d : public texture {
public:
    checker_3d() {}

    checker_3d(double x, double y, color c1, color c2) : dx(x), dy(y), col1(c1), col2(c2) {}

    virtual color value(double u, double v, const vec3& p) const override {
        double eps = 0.000001;
        if (((int) (floor(p.x() + eps) + floor(p.y() + eps) + floor(p.z() + eps))) % 2) {
            return col1;
        } else {
        return col2;
        }
    }

private:
    double dx = 1;
    double dy = 1;
    color col1 = color(0,0,0);
    color col2 = color(1,1,1);
};

class striped_sphere : public texture {
public:
    striped_sphere() {}
    striped_sphere(int s, color c1, color c2) : stripecount(s), col1(c1), col2(c2) {}

    virtual color value(double u, double v, const vec3& p) const override {
        if ((int) (v * stripecount) % 2) {
            return col1;
        }
        else {
            return col2;
        }
    }

private:
    int stripecount = 8;
    color col1 = color(0, 0, 0);
    color col2 = color(1, 1, 1);
};

class checkered_sphere : public texture {
public:
    checkered_sphere() {}
    checkered_sphere(int s, color c1, color c2) : stripecount(s), col1(c1), col2(c2) {}

    virtual color value(double u, double v, const vec3& p) const override {
        if (((int)(v * stripecount) + (int)(u * stripecount)) % 2) {
            return col1;
        }
        else {
            return col2;
        }
    }

private:
    int stripecount = 8;
    color col1 = color(0, 0, 0);
    color col2 = color(1, 1, 1);
};

// Uses perlin noise to create a texture based on position.
class perlin_meat : public texture {
public:
    perlin_meat() {}

    virtual color value(double u, double v, const vec3& p) const override {
        double perlinval = ImprovedNoise::noise(p.x(), p.y(), p.z());

        if (perlinval < t0) {
            return c0;
        }
        else if (perlinval < t1) {
            double interval = t1 - t0;
            return (t1 - perlinval) / interval * c0 + (perlinval - t0) / interval * c1;
        }
        else if (perlinval < t2) {
            double interval = t2 - t1;
            return (t2 - perlinval) / interval * c1 + (perlinval - t1) / interval * c2;
        }
        else {
            return c2;
        }

    }
private:
    color c0 = color(0.305, 0.010, 0.010);
    color c1 = color(0.644, 0.004, 0.003);
    color c2 = color(0.965, 0.644, 0.761);

    double t0 = 0.220;
    double t1 = 0.423;
    double t2 = 0.614;
};

#endif
