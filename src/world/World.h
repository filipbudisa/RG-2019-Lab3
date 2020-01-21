#ifndef VULK_WORLD_H
#define VULK_WORLD_H


#include <vector>
#include "WorldObject.h"
#include "../interfaces/ITimeBound.h"
#include "../particlesystem/ParticleSystem.h"
#include "../springsystem/SpringSystem.h"

class World : ITimeBound {
public:
	World();

	void load(int i);
	void load1();
	void load2();
	void load3();
	void load4();
	void load5();
	void load6();
	void cleanup();
	virtual void update(double time);

private:
	WorldObject * loadModel(const Mesh mesh, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale);
	void loadModel(const Mesh mesh);
	void updateTransformationmatrices();
	std::vector<ParticleSystem> pSystems;
	std::vector<SpringSystem*> sSystems;
	unsigned scene;
};


#endif //VULK_WORLD_H
