#ifndef VULK_MESH_H
#define VULK_MESH_H


#include <vector>
#include "Vulkan.h"

class Mesh {
public:
	Mesh(const std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices);

	static Mesh generateSphere(float r, int sectorCount, int stackCount);
	static Mesh generatePlane(glm::vec2 start, glm::vec2 end, int n);
	static Mesh generatePlane(glm::vec2 start, glm::vec2 end, int n, bool alt);
	static Mesh load(const std::string &filename);

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
};


#endif //VULK_MESH_H
