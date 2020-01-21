#include "Storage.h"
#include "../springsystem/SpringSystem.h"

std::vector<RenderComponent*> Storage::renderObjects;
std::vector<WorldObject*> Storage::worldObjects;
std::vector<Bspline*> Storage::splines;
std::vector<Particle*> Storage::particles;
std::vector<Spring*> Storage::springs;
std::vector<SpringSystem*> Storage::sSystems;
std::array<std::vector<RenderComponent*>, 2> Storage::renderObjectGarbage;

int Storage::frame = 0;

Graphics *Storage::graphics;

void Storage::init(Graphics *graphics){
	Storage::graphics = graphics;
}

void Storage::addRenderObject(RenderComponent *rObj){
	graphics->regObject(rObj);
	renderObjects.push_back(rObj);
}

void Storage::removeRenderObject(RenderComponent *rObj){
	renderObjectGarbage[frame].push_back(rObj);
	renderObjects.erase(std::remove(renderObjects.begin(), renderObjects.end(), rObj), renderObjects.end());
}

void Storage::removeWorldObject(WorldObject *wObj){
	worldObjects.erase(std::remove(worldObjects.begin(), worldObjects.end(), wObj), worldObjects.end());
	delete wObj;
}

void Storage::addParticle(Particle *p){
	graphics->regObject(p->getWObject()->renderComponent);
	particles.push_back(p);
}

void Storage::removeParticle(Particle *p){
	renderObjectGarbage[frame].push_back(p->getWObject()->renderComponent);
	particles.erase(std::remove(particles.begin(), particles.end(), p), particles.end());
	Storage::removeWorldObject(p->getWObject());
	delete p;
}

void Storage::clearGarbage(){
	frame = (frame+1) % 2;

	for(RenderComponent* rObj : renderObjectGarbage[frame]){
		graphics->deregObject(rObj);
		delete rObj;
	}

	renderObjectGarbage[frame].clear();
}

void Storage::cleanup(){
	for(RenderComponent* rObj : renderObjects){
		graphics->deregObject(rObj);
		delete rObj;
	}

	for(WorldObject* wObj : worldObjects){
		wObj->cleanup();
		delete wObj;
	}

	for(SpringSystem* system : sSystems){
		graphics->deregMass(system);
		system->cleanup();
		delete system;
	}

	/*for(Bspline* s : splines){
		graphics->deregSpline(s);
	}

	for(Particle* p : particles){
		graphics->deregObject(p->getWObject()->renderComponent);
	}

	for(Spring* s : springs){
		graphics->deregSpring(s);
	}*/

	renderObjects.clear();
	worldObjects.clear();
	/*splines.clear();
	particles.clear();*/
	springs.clear();
	sSystems.clear();
}

void Storage::dereg(){

}
