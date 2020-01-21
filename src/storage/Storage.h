#ifndef VULK_STORAGE_H
#define VULK_STORAGE_H


#include "../graphics/RenderComponent.h"
#include "../world/WorldObject.h"
#include "../graphics/Graphics.h"
#include "../curves/Bspline.h"
#include "../particlesystem/Particle.h"
#include "../springsystem/MassPoint.h"
#include "../springsystem/Spring.h"
#include "../springsystem/SpringSystem.h"

class Storage {
public:
	static void init(Graphics *graphics);

	static std::vector<RenderComponent*> renderObjects;
	static std::vector<WorldObject*> worldObjects;
	static std::vector<Bspline*> splines;
	static std::vector<Particle*> particles;
	static std::vector<Spring*> springs;
	static std::vector<SpringSystem*> sSystems;

	static void addRenderObject(RenderComponent *rObj);
	static void removeRenderObject(RenderComponent *rObj);

	static void clearGarbage();
	static void cleanup();
	static void dereg();

	static void addParticle(Particle* p);
	static void removeParticle(Particle* p);

	static void removeWorldObject(WorldObject *wObj);

private:
	static Graphics *graphics;

	static int frame;
	static std::array<std::vector<RenderComponent*>, 2> renderObjectGarbage;
};


#endif //VULK_STORAGE_H
