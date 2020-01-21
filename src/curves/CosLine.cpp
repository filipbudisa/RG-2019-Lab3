#include "CosLine.h"

void CosLine::update(double time){
	float delta = cos(2.0f * M_PI * (totalTime + time) / period) - cos(2.0f * M_PI * totalTime / period);
	totalTime += time;

	object->position += influences * amplitude * delta;
}

CosLine::CosLine(WorldObject *obj, const glm::vec3& influences, float amplitude, float period) : IObjectModifier(obj),
																								 influences(influences),
																								 amplitude(amplitude),
																								 period(period){}
