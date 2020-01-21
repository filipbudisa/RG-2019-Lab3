#ifndef VULK_PLAYERMOVEMENT_H
#define VULK_PLAYERMOVEMENT_H

#include "Player.h"

class Player;

enum PlayerMovementDirection {
	STOP,
	MOVING_FORWARD = 1,
	MOVING_BACKWARD = MOVING_FORWARD << 1,
	STRAFING_LEFT = MOVING_FORWARD << 2,
	STRAFING_RIGH = MOVING_FORWARD << 3,
	MOVING_UP = MOVING_FORWARD << 4,
	MOVING_DOWN = MOVING_FORWARD << 5,
};

class PlayerMovement {
public:
	PlayerMovement(Player* player);
	void addDirection(PlayerMovementDirection _direction);
	void removeDirection(PlayerMovementDirection _direction);
	void update(double time);

private:
	Player* player;
	int direction = 0;
};


#endif //VULK_PLAYERMOVEMENT_H
