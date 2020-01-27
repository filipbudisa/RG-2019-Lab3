#ifndef VULK_WORLD_H
#define VULK_WORLD_H


#include <vector>
#include "WorldObject.h"
#include "../interfaces/ITimeBound.h"
#include "../springsystem/SpringSystem.h"

class World : ITimeBound {
public:
	World();

	void load(int scene);
	void load1();
	void load2();
	void load3();
	void cleanup();
	virtual void update(double time);

private:
	WorldObject * loadModel(const Mesh mesh, glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale);
	void loadModel(const Mesh mesh);
	void updateTransformationmatrices();
};


#endif //VULK_WORLD_H
