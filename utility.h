#ifndef UTILITY_H
#define UTILITY_H

#include <cmath>
#include <cstdlib>
#include <limits>
#include <memory>

#include "vec3.h"

// Usings

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// Constants

const double db_max = std::numeric_limits<double>::max();
const double db_min = std::numeric_limits<double>::min();
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;
const double epsilon = 0.00001;
const color default_color = color(1, 0.65, 0);
const double transparency_inner = 0.8;

// Utility Functions
inline int random_int(int min, int max) {
    return min + (rand() % static_cast<int>(max - min + 1));
}

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double clamp(double x, double min, double max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

// Common Headers

#include "ray.h"
#include "vec3.h"

#endif