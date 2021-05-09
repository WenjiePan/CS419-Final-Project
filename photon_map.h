#ifndef PHOTON_MAP_H
#define PHOTON_MAP_H

#include "photon.h"
#include "utility.h"
#include <algorithm>
#include <vector>

using namespace std;

class photon_map {
	public:
		std::vector<photon*> photons;

	private:
		double cosphi[256];
		double sinphi[256];
		double costheta[256];
		double sintheta[256];

	public:
		photon_map() {};
		photon_map(std::vector<photon*>& p) {
			for (int i = 0; i < 256; i++) {
				double angle = double(i) * (1.0 / 255.0) * pi;
				costheta[i] = cos(angle);
				sintheta[i] = sin(angle);
				cosphi[i] = cos(2.0 * angle);
				sinphi[i] = sin(2.0 * angle);
			}

			photons.resize(p.size());
			balance(p, 0, p.size(), 0);

			// Print test
			//for (int i = 0; i < 10; i++) {
			//	printf("Pos: %.2f, %.2f, %.2f\n", photons[i]->pos.x(), photons[i]->pos.y(), photons[i]->pos.z());
			//	printf("Pow: %.2f, %.2f, %.2f\n", photons[i]->pow.x(), photons[i]->pow.y(), photons[i]->pow.z());
			//	printf("\n");
			//}
		}

		void balance(std::vector<photon*>& buf, int left, int right, int index) {
			if (right - left == 1) {
				photons[index] = buf[left];
				photons[index]->flag = 0;
				return;
			}
			else if (right - left == 0) { return; }

			aabb cube = aabb();
			for (int i = left; i < right; i++) {
				cube.fit(buf[i]->pos);
			}
			int dim = cube.sepAxis();
			int median = left + median_func(right - left);
			std::nth_element(buf.begin() + left, buf.begin() + median, buf.begin() + right, (dim == 0) ? compX : (dim == 1) ? compY : compZ);
			
			photon* med_photon = buf[median];
			med_photon->flag = dim;
			photons[index] = med_photon;

			balance(buf, left, median, index * 2 + 1);
			balance(buf, median + 1, right, index * 2 + 2);
		}

		void nearest(std::vector<photon*>& neighbors, std::vector<double>& dists, point3 hit, int estimate, int index) {
			if (index >= photons.size()) { return; }
			else if (index * 2 + 1 < photons.size()) {
				double curr_max_dist = dists[estimate - 1];
				int dim = photons[index]->flag;
				double d_diff = (dim == 0 ? hit.x() - photons[index]->pos.x() : (dim == 1 ? hit.y() - photons[index]->pos.y() : hit.z() - photons[index]->pos.z()));
				if (d_diff < 0) {
					nearest(neighbors, dists, hit, estimate, index * 2 + 1);
					if (d_diff * d_diff < curr_max_dist) {
						nearest(neighbors, dists, hit, estimate, index * 2 + 2);
					}
				}
				else {
					nearest(neighbors, dists, hit, estimate, index * 2 + 2);
					if (d_diff * d_diff < curr_max_dist) {
						nearest(neighbors, dists, hit, estimate, index * 2 + 1);
					}
				}
			}

			double d_diff2 = (photons[index]->pos - hit).length_squared();
			if (d_diff2 > dists[estimate - 1]) { return; }
			int left, right, middle;
			for (left = 0, right = estimate; left < right; ) {
				middle = (left + right) / 2;
				if (d_diff2 < dists[middle])
					right = middle;
				else
					left = middle + 1;
			}

			for (int i = estimate - 1; i > left; i--) {
				neighbors[i] = neighbors[i - 1];
				dists[i] = dists[i - 1];
			}
			neighbors[left] = photons[index];
			dists[left] = d_diff2;
		}

	private:
		static bool compX(const photon* a, const photon* b) { return a->pos.x() < b->pos.x(); }
		static bool compY(const photon* a, const photon* b) { return a->pos.y() < b->pos.y(); }
		static bool compZ(const photon* a, const photon* b) { return a->pos.z() < b->pos.z(); }

		int median_func(int n) {
			int log_2n = 0;
			int pow_2 = 1;
			while (n / pow_2 > 1) {
				pow_2 *= 2;
				log_2n += 1;
			}

			int s = 1 << log_2n;
			int diff = n - s;
			int half_diff = s / 2;
			if (half_diff > 0 && diff >= half_diff) {
				diff = half_diff - 1;
			}
			return half_diff + diff;
		}
};

void photon_tracing(hittable_list& world, vector<photon*>& photons, ray& photon_ray, color ray_color, int depth) {
	if (depth <= 0) { return; }

	depth -= 1;
	hit_record rec;
	ray scattered;
	color attenuation;
	color new_ray_color;
	if (!world.hit(photon_ray, epsilon, infinity, rec)) { return; }

	color hit_color = rec.mat_ptr->getColor();
	double p_diffuse = maximum_element(hit_color);

	if (rec.mat_ptr->getMatName().compare("diffuse") == 0) {
		photon* p = new photon;
		p->pos = rec.p;
		p->pow = ray_color * hit_color;
		vec3 unit_dir = unit_vector(photon_ray.direction());
		p->dir = unit_dir;
		p->flag = 0;
		photons.push_back(p);

		if (random_double() > p_diffuse) { return; }
		else {
			if (!rec.mat_ptr->scatter(photon_ray, rec, attenuation, scattered)) { return; }
			else {
				//new_ray_color = hit_color * ray_color * (1.0f / p_diffuse);
				new_ray_color = hit_color * ray_color;
				photon_tracing(world, photons, scattered, new_ray_color, depth);
			}
		}
	}
	else if (rec.mat_ptr->getMatName().compare("specular") == 0) {
		if (!rec.mat_ptr->scatter(photon_ray, rec, attenuation, scattered)) { return; }
		else {
			new_ray_color = hit_color * ray_color;
			photon_tracing(world, photons, scattered, new_ray_color, depth);
		}
	}
	else if (rec.mat_ptr->getMatName().compare("refract") == 0) {
		if (!rec.mat_ptr->scatter(photon_ray, rec, attenuation, scattered)) { return; }
		else {
			new_ray_color = hit_color * ray_color;
			photon_tracing(world, photons, scattered, new_ray_color, depth);
		}
	}
}

#endif // !PHOTON_MAP_H
