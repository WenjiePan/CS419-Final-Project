#ifndef OBJ_H
#define OBJ_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "utility.h"
#include "triangle.h"

// Class for parsing obj file and computing per-vertex normals of obj file
class obj {
	public:
		std::string fileName;
		std::vector<point3> vertices;
		std::vector<shared_ptr<triangle>> meshes;
		std::vector<vec3> normals;

	public:
		obj() {}
		obj(std::string filePath) {
			fileName = filePath;
			std::ifstream file(filePath);
			std::string line;

			// Iterate through the file line by line for parsing
			while (std::getline(file, line)) {
				// If line has vertex info
				if (line.find("v") == 0) {
					std::istringstream iss(line);
					std::vector<std::string> parsed_double((
						std::istream_iterator<std::string>(iss)),
						std::istream_iterator<std::string>());
					double x = std::stod(parsed_double[1]);
					double y = std::stod(parsed_double[2]);
					double z = std::stod(parsed_double[3]);

					// Parse vertex position and push to vector
					vertices.push_back(point3(x, y, z));
					// Initialize per-vertex normal for the parsed vertex
					normals.push_back(vec3(0, 0, 0));
				}

				// If line has face info
				else if (line.find("f") == 0) {
					std::istringstream iss(line);
					std::vector<std::string> parsed_int((
						std::istream_iterator<std::string>(iss)),
						std::istream_iterator<std::string>());
					int v0 = std::stoi(parsed_int[1]) - 1;
					int v1 = std::stoi(parsed_int[2]) - 1;
					int v2 = std::stoi(parsed_int[3]) - 1;

					// Construct triangle with parsed face info
					shared_ptr<triangle> tri = make_shared<triangle>(vertices[v0], vertices[v1], vertices[v2], default_color);
					vec3 n = tri->getFaceNormal();
					// Compute vertex normal for each vertex
					normals[v0] += n;
					normals[v1] += n;
					normals[v2] += n;
					// Push triangle to vector
					meshes.push_back(tri);
				}

				else
					continue;
			}

			// Normalize all per-vertex normals
			for (int i = 0; i < normals.size(); i++) {
				normals[i] = unit_vector(normals[i]);
			}

			file.close();
		}

		std::vector<point3> getVertices() { return vertices; }
		std::vector<shared_ptr<triangle>> getMeshes() { return meshes; }

		void update_vertex_normals();
};

// Update all vertex normals in the triangles
void obj::update_vertex_normals() {
	std::ifstream file(fileName);
	std::string line;
	int tri_count = 0;

	while (std::getline(file, line)) {
		if (line.find("f") == 0) {
			std::istringstream iss(line);
			std::vector<std::string> parsed_int((
				std::istream_iterator<std::string>(iss)),
				std::istream_iterator<std::string>());
			int v0 = std::stoi(parsed_int[1]) - 1;
			int v1 = std::stoi(parsed_int[2]) - 1;
			int v2 = std::stoi(parsed_int[3]) - 1;

			// Update per-vertex normals in the triangle
			meshes[tri_count]->normal_v0 = normals[v0];
			meshes[tri_count]->normal_v1 = normals[v1];
			meshes[tri_count]->normal_v2 = normals[v2];

			tri_count++;
		}
	}
}

#endif OBJ_H