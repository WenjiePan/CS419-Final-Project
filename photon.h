#ifndef PHOTON_H
#define PHOTON_H

#include "vec3.h"

struct photon {
	vec3 pos = vec3(0, 0, 0);					// position
	color pow = vec3(0, 0, 0);					// power(RGB color)
	vec3 dir = vec3(0, 0, 0);					// incident direction
	short flag = 0;								// flag used in kdtree
};

#endif