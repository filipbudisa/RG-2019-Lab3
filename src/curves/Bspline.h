#ifndef VULK_BSPLINE_H
#define VULK_BSPLINE_H


#include "../interfaces/IObjectModifier.h"

class Bspline : public ITimeBound {
public:
	Bspline(const std::string& description, double duration);
	virtual void update(double time);

	BufferAllocation *vertexBuffer = nullptr;
	BufferAllocation *directionBuffer = nullptr;
	std::vector<Vertex> vertices;
	std::vector<Vertex> direction;
	glm::mat4 transforms;

	glm::mat4 tNormal;
	glm::mat4 tVert;

private:
	void load(const std::string& path);
	void calcSegments();
	void calcVertices();

	double duration;
	double curTime;
	bool reverse = false;
	std::vector<glm::vec4> points;
	std::vector<glm::mat4> segments;
	std::vector<glm::mat4x3> dSegments;
};


#endif //VULK_BSPLINE_H
