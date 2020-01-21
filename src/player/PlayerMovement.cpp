#include "PlayerMovement.h"
#include "Player.h"
#include "../graphics/Camera.h"
#include "../graphics/Graphics.h"


PlayerMovement::PlayerMovement(Player *player) : player(player){

}

void PlayerMovement::update(double time){
	Camera* camera = player->camera;
	glm::vec3 cameraDirection = camera->getDirection();
	glm::vec3 movementDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 right = glm::normalize(glm::cross(cameraDirection, UP));

	if(direction & MOVING_FORWARD){
		movementDirection += cameraDirection;
	}

	if(direction & MOVING_BACKWARD){
		movementDirection -= cameraDirection;
	}

	if(direction & STRAFING_RIGH){
		movementDirection += right;
	}

	if(direction & STRAFING_LEFT){
		movementDirection -= right;
	}

	if(direction & MOVING_UP){
		movementDirection += UP;
	}

	if(direction & MOVING_DOWN){
		movementDirection -= UP;
	}

	if(!(movementDirection.x == movementDirection.y == movementDirection.z == 0.0f)){
		movementDirection = glm::normalize(movementDirection);
	}

	time *= 3.0f;

	camera->setX(camera->getX() + time * movementDirection.x);
	camera->setY(camera->getY() + time * movementDirection.y);
	camera->setZ(camera->getZ() + time * movementDirection.z);
}

void PlayerMovement::addDirection(PlayerMovementDirection _direction){
	direction |= _direction;
}

void PlayerMovement::removeDirection(PlayerMovementDirection _direction){
	direction &= ~_direction;
}
