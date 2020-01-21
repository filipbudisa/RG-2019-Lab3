
#include <iostream>
#include "Camera.h"
#include "Graphics.h"

Camera::Camera(float x, float y, float z, float pitch, float yaw, float roll) : x(x), y(y), z(z), pitch(pitch),
																				yaw(yaw), roll(roll){


}

glm::vec3 Camera::getDirection(){
	glm::mat4 matRoll  = glm::rotate(glm::mat4(1.0f), -getRoll(),  glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 matPitch = glm::rotate(glm::mat4(1.0f), -getPitch(), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 matYaw = glm::rotate(glm::mat4(1.0f), -getYaw(), glm::vec3(0.0f, 0.0f, 1.0f));

	glm::mat4 rotate = matYaw * matPitch;

	return glm::normalize(rotate * FORWARD);
}

glm::vec3 Camera::getPosition(){
	return { x, y, z };
}

float Camera::getX() const{
	return x;
}

void Camera::setX(float x){
	Camera::x = x;
}

float Camera::getY() const{
	return y;
}

void Camera::setY(float y){
	Camera::y = y;
}

float Camera::getZ() const{
	return z;
}

void Camera::setZ(float z){
	Camera::z = z;
}

float Camera::getPitch() const{
	return pitch;
}

void Camera::setPitch(float pitch){
	pitch = glm::min<float>(pitch, glm::half_pi<float>());
	pitch = glm::max<float>(pitch, -glm::half_pi<float>());

	Camera::pitch = pitch;
}

float Camera::getYaw() const{
	return yaw;
}

void Camera::setYaw(float yaw){
	if(yaw >= 2.0f*3.14f){
		yaw = yaw - glm::two_pi<float>();
	}else if(yaw <= 0.0f){
		yaw = glm::two_pi<float>() + yaw;
	}

	Camera::yaw = yaw;
}

float Camera::getRoll() const{
	return roll;
}

void Camera::setRoll(float roll){
	Camera::roll = roll;
}
