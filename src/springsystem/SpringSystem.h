#ifndef VULK_SPRINGSYSTEM_H
#define VULK_SPRINGSYSTEM_H


#include <vector>
#include "Spring.h"
#include "../physics/IPhysicsComponent.h"

class Spring;
class WorldObject;

class SpringSystem : public IPhysicsComponent {
public:
	SpringSystem(WorldObject *object, unsigned n, float mass);

	void update(double time) override;
	void resetForce() override;
	void collide(CollisionComponent* collidor) override;

	void addSpring(Spring* spring);
	void addPoint(MassPoint* point);
	void setMesh(const std::vector<std::vector<MassPoint *>>& mesh);
	const std::vector<std::vector<MassPoint *>>& getMesh() const;
	int getNoPoints();
	MassPoint* getPoint(int n);
	void setFixed(int i, int j, bool fixed);

	void cleanup();

	WorldObject* object;
private:
	unsigned n;

	void constructPoints(float mass);
	void constructSprings();

	std::vector<MassPoint*> points;
	std::vector<Spring*> springs;
	std::vector<std::vector<MassPoint*>> mesh;
};


#endif //VULK_SPRINGSYSTEM_H
