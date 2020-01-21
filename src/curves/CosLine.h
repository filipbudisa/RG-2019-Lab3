#ifndef VULK_COSLINE_H
#define VULK_COSLINE_H

#include <glm/vec3.hpp>
#include "../interfaces/IObjectModifier.h"

class CosLine : public IObjectModifier {
public:
	CosLine(WorldObject *obj, const glm::vec3& influences, float amplitude, float period);

	virtual void update(double time);

private:
	glm::vec3 influences;
	float amplitude;
	float period;

	float totalTime = 0;
};


#endif //VULK_COSLINE_H
