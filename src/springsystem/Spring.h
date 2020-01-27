#ifndef VULK_SPRING_H
#define VULK_SPRING_H


#include <utility>
#include "../interfaces/ITimeBound.h"
#include "MassPoint.h"
#include "../graphics/BufferAllocation.h"
#include "../graphics/Vulkan.h"
#include "SpringSystem.h"

class SpringSystem;

class Spring : public ITimeBound {
public:
	Spring(uint16_t a, uint16_t b, float k);

	BufferAllocation *vertexBuffer = nullptr;
	std::vector<Vertex> vertices;

	void update(double time) override;

	void updateVertices();

	void setSystem(SpringSystem *system);

	const std::pair<uint16_t, uint16_t>& getIndexes() const;

	float k;
	float length;


private:
	SpringSystem* system;
	std::pair<uint16_t, uint16_t> pointsIndexes;
	std::pair<MassPoint*, MassPoint*> points;
};


#endif //VULK_SPRING_H
