#include <glm/vec3.hpp>
#include "Player.h"
#include "PlayerMovement.h"

Player::Player(Camera* _camera) : camera(_camera), movement(new PlayerMovement(this)){

}

void Player::input(int code, bool on){
	switch(code){
		case 0:
			break;
		case 1:
			if(on) movement->addDirection(MOVING_FORWARD);
			else movement->removeDirection(MOVING_FORWARD);
			break;
		case 2:
			if(on) movement->addDirection(MOVING_BACKWARD);
			else movement->removeDirection(MOVING_BACKWARD);
			break;
		case 3:
			if(on) movement->addDirection(STRAFING_LEFT);
			else movement->removeDirection(STRAFING_LEFT);
			break;
		case 4:
			if(on) movement->addDirection(STRAFING_RIGH);
			else movement->removeDirection(STRAFING_RIGH);
			break;
		case 5:
			if(on) movement->addDirection(MOVING_UP);
			else movement->removeDirection(MOVING_UP);
			break;
		case 6:
			if(on) movement->addDirection(MOVING_DOWN);
			else movement->removeDirection(MOVING_DOWN);
			break;
	}
}

void Player::cursor(double x, double y){
	camera->setYaw(camera->getYaw() + x / 300.0f);
	camera->setPitch(camera->getPitch() + y / 300.0f);
}

void Player::update(double time){
	movement->update(time);
}
