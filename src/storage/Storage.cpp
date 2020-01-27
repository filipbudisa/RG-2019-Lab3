#include "Storage.h"
#include "../springsystem/SpringSystem.h"

std::vector<RenderComponent*> Storage::renderObjects;
std::vector<WorldObject*> Storage::worldObjects;
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

	for(int i = 0; i < Storage::springs.size(); i++){
		graphics->deregSpring(Storage::springs[i]);
	}

	for(RenderComponent* rObj : renderObjects){
		graphics->deregObject(rObj);
		delete rObj;
	}

	for(WorldObject* wObj : worldObjects){
		wObj->cleanup();
		delete wObj;
	}

	renderObjects.clear();
	worldObjects.clear();
	springs.clear();
	sSystems.clear();
}
