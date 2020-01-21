#ifndef VULK_PHYSICSENGINE_H
#define VULK_PHYSICSENGINE_H


#include <vector>
#include "../interfaces/ITimeBound.h"
#include "CollisionComponent.h"
#include "StandardPhysicsComponent.h"
#include "../graphics/Graphics.h"

#define TIME_DELTA 0.001

class PhysicsEngine : public ITimeBound {
public:
	void update(double time) override;
	void update(double time, Graphics* graphics);


	static std::vector<CollisionComponent*> colComps;
	static std::vector<IPhysicsComponent*> physComps;

	static void cleanup();

private:
	double timeResidue = 0;
};


#endif //VULK_PHYSICSENGINE_H
