#ifndef VULK_CAMERA_H
#define VULK_CAMERA_H


#include <glm/glm.hpp>

class Camera {
public:
	Camera(float x, float y, float z, float pitch, float yaw, float roll);

	glm::vec3 getDirection();

	glm::vec3 getPosition();

	float getX() const;

	void setX(float x);

	float getY() const;

	void setY(float y);

	float getZ() const;

	void setZ(float z);

	float getPitch() const;

	void setPitch(float pitch);

	float getYaw() const;

	void setYaw(float yaw);

	float getRoll() const;

	void setRoll(float roll);

private:
	float x, y, z;
	float pitch, yaw, roll;
};


#endif //VULK_CAMERA_H
