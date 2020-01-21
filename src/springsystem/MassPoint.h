#ifndef VULK_MASSPOINT_H
#define VULK_MASSPOINT_H


#include <glm/vec3.hpp>
#include "../interfaces/ITimeBound.h"
#include "../graphics/Vulkan.h"
#include "../graphics/RenderComponent.h"

class MassPoint : public ITimeBound {
public:
	MassPoint(Vertex* vertex, MeshTransforms* transforms, float mass);

	void resetForce();
	void addForce(glm::vec3 force);
	void addVelocity(glm::vec3 velocity);

	void update(double time) override;

	glm::vec3 getPosition() const;

	const glm::vec3& getVelocity() const;

	void setPosition(const glm::vec3& position);
	void move(const glm::vec3& direction);

	float getMass() const;

	void setFixed(bool fixed);

private:
	//glm::vec3 position;
	Vertex* vertex;
	MeshTransforms* transforms;
	glm::vec3 posLast = { 0, 0, 0 };
	glm::vec3 velLast = { 0, 0, 0 };
	glm::vec3 velocity = { 0, 0, 0 };
	glm::vec3 force;
	bool fixed = false;
	float mass = 0;
};


#endif //VULK_MASSPOINT_H
