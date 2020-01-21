
#ifndef VULK_GAME_H
#define VULK_GAME_H


#include "world/World.h"
#include "player/Player.h"
#include "graphics/Graphics.h"
#include "physics/PhysicsEngine.h"

#define TICK 100

typedef std::chrono::high_resolution_clock Clock;

extern bool drawMesh;

class Game {
public:
	void init(int scene);
	void run();
	void loadScene(int i);
private:
	void updateLogic();

	Clock::time_point time;

	World* world;
	Player* player;
	Graphics* graphics;
	PhysicsEngine* physics;
};


#endif //VULK_GAME_H
