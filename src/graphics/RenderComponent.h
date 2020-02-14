#ifndef VULK_RENDERCOMPONENT_H
#define VULK_RENDERCOMPONENT_H


#include <cstdint>
#include <vector>
#include "Vulkan.h"
#include "Mesh.h"
#include "BufferAllocation.h"

struct MeshTransforms {
	glm::mat4 tObject;
	glm::mat4 tNormal;
};

class RenderComponent {
public:
	RenderComponent(const Mesh &mesh);

	void calculateNormals();

	static MeshTransforms identity;

	Mesh mesh;

	MeshTransforms transforms;
	BufferAllocation *vertexBuffer = nullptr;

	BufferAllocation *indexBuffer = nullptr;

	unsigned pipeline;
private:
};


#endif //VULK_RENDERCOMPONENT_H
