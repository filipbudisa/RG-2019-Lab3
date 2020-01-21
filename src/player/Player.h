#ifndef VULK_PLAYER_H
#define VULK_PLAYER_H

#include "../graphics/Camera.h"
#include "PlayerMovement.h"

class PlayerMovement;

class Player {
public:
	Player(Camera* _camera);
	void input(int code, bool on);
	void cursor(double x, double y);
	void update(double time);

	Camera* camera;

private:
	PlayerMovement* movement;
};


#endif //VULK_PLAYER_H
