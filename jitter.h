#ifndef JITTER_H
#define JITTER_H

#include "utility.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <iostream>
#include <vector>

using std::vector;

// Class for building multi-jittered sampling for anti-aliasing purpose
class jitter {
	private:
		int grid;
		int coarse_grid;
		double grid_unit;
		double coarse_grid_unit;

	public:
		jitter(int g) {
			grid = g;
			coarse_grid = static_cast<int> (std::sqrt((double)grid));
			grid_unit = 1.0 / grid;
			coarse_grid_unit = 1.0 / coarse_grid;
		}

		int get_grid() const {
			return grid;
		}

		int get_coarse_grid() const {
			return coarse_grid;
		}

		// Get all the possible sample points given row and column
		vector<vector<double>> get_samples(int row, int col) {
			vector<vector<double>> sample_points;
			vector<vector<int>> row_ceils;
			vector<vector<int>> col_ceils;

			// Construct row and column index vector
			for (int i = 0; i < coarse_grid; i++) {
				vector<int> line;
				for (int j = 0; j < coarse_grid; j++) {
					line.push_back(j);
				}
				row_ceils.push_back(line);
				col_ceils.push_back(line);
			}

			// Create sample point using multi-jittered sampling technique
			for (int i = 0; i < coarse_grid; i++) {
				for (int j = 0; j < coarse_grid; j++) {
					vector<double> sample_point;
					// Randomly pick 1 unique index in the row
					int row_idx = random_int(0, row_ceils[i].size() - 1);
					double row_min = row + i * coarse_grid_unit + row_ceils[i][row_idx] * grid_unit;
					// Randomly pick 1 point among the grid range in the row
					sample_point.push_back(random_double(row_min, row_min + grid_unit));
					// Randomly pick 1 unique index in the column
					int col_idx = random_int(0, col_ceils[j].size() - 1);
					double col_min = col + j * coarse_grid_unit + col_ceils[j][col_idx] * grid_unit;
					// Randomly pick 1 point among the grid range in the column
					sample_point.push_back(random_double(col_min, col_min + grid_unit));

					// Delete the chosen index in row and column so that it would not be chosen later
					row_ceils[i].erase(row_ceils[i].begin() + row_idx);
					col_ceils[j].erase(col_ceils[j].begin() + col_idx);

					// Push the generate sample point to the results
					sample_points.push_back(sample_point);
				}
			}

			//std::cout << "Sample for: " << row << ", " << col << "\n";
			//for (int i = 0; i < sample_points.size(); i++) {
			//	for (int j = 0; j < sample_points[i].size(); j++) {
			//		std::cout << sample_points[i][j] << " ";
			//	}
			//	std::cout << "\n";
			//}

			return sample_points;
		}
};

#endif
