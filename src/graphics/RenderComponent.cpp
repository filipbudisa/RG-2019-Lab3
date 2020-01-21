#include "RenderComponent.h"
#include "../storage/Storage.h"

MeshTransforms RenderComponent::identity({glm::mat4(1.0f), glm::mat4(1.0f) });

RenderComponent::RenderComponent(const Mesh &mesh) : mesh(mesh), transforms({glm::mat4(1.0f), glm::mat4(1.0f) }){
	Storage::renderObjects.push_back(this);

	pipeline = 0;

	calculateNormals();
}

void RenderComponent::calculateNormals(){
	for(int i = 0; i < mesh.indices.size(); i += 3){
		glm::vec3 vertA = mesh.vertices[mesh.indices[i+0]].pos;
		glm::vec3 vertB = mesh.vertices[mesh.indices[i+1]].pos;
		glm::vec3 vertC = mesh.vertices[mesh.indices[i+2]].pos;

		glm::vec3 lineA = vertB - vertA;
		glm::vec3 lineB = vertC - vertB;

		glm::vec3 faceNormal = glm::cross(lineA, lineB);

		mesh.vertices[mesh.indices[i+0]].normal += faceNormal;
		mesh.vertices[mesh.indices[i+1]].normal += faceNormal;
		mesh.vertices[mesh.indices[i+2]].normal += faceNormal;
	}

	for(int i = 0; i < mesh.vertices.size(); i++){
		mesh.vertices[i].normal = glm::normalize(mesh.vertices[i].normal);
	}
}
