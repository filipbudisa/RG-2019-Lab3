#include <sstream>
#include "Bspline.h"
#include "../graphics/Graphics.h"
#include <glm/gtx/matrix_interpolation.hpp>

Bspline::Bspline(const std::string& description, double duration)
		: duration(duration), curTime(0), transforms(glm::mat4(1.0f)){

	load(description);
	calcSegments();
	calcVertices();

}

void Bspline::load(const std::string& path){
	std::ifstream file(path);
	std::string line;
	while(std::getline(file, line)){
		std::stringstream lineStream(line);
		glm::vec4 point;
		lineStream >> point.x >> point.y >> point.z;
		point.w = 1.0f;
		points.push_back(point);
	}
}

void Bspline::calcSegments(){
	glm::mat4 mat = {
			{ -1.0f, 3.0f, -3.0f, 1.0f },
			{ 3.0f, -6.0f, 3.0f, 0.0f },
			{ -3.0f, 0.0f, 3.0f, 0.0f },
			{ 1.0f, 4.0f, 1.0f, 0.0f }
	};

	mat /= 6.0f;
	mat = glm::transpose(mat);

	glm::mat3x4 dMata = {
			{ -1.0f, 3.0f, -3.0f, 1.0f },
			{ 2.0f, -4.0f, 2.0f, 0.0f },
			{ -1.0f, 0.0f, 1.0f, 0.0f }
	};

	dMata /= 2.0f;
	glm::mat4x3 dMat = glm::transpose(dMata);

	for(int i = 0; i < points.size()-3; i++){
		glm::mat4 segment = mat * glm::transpose(glm::mat4(points[i], points[i+1], points[i+2], points[i+3]));
		segments.push_back(segment);

		glm::mat4x3 dSegment = dMat * glm::transpose(glm::mat4(points[i], points[i+1], points[i+2], points[i+3]));
		dSegments.push_back(dSegment);
	}
}

void Bspline::calcVertices(){
	int k = 0;
	for(glm::mat4 segment : segments){
		k++;
		for(int i = 0; i <= 10; i++){
			float t = i/10.0f;
			glm::vec4 timeVec(pow(t, 3), pow(t, 2), t, 1.0f);
			glm::vec4 pos = timeVec*segment;

			Vertex v;
			v.pos = glm::vec3(pos / pos.w);
			v.color = glm::vec3(1.0f - 0.1 * (float) k, 0.1f * (float) k, 0.0f	);
			vertices.push_back(v);
		}
	}

	Vertex a; a.pos = glm::vec3(0.0f, 0.0f, 0.0f); a.color = glm::vec3(0.0f, 0.5f, 1.0f);
	Vertex b; b.pos = glm::vec3(0.0f, 100.0f, 0.0f); b.color = glm::vec3(0.0f, 0.5f, 1.0f);
	direction.push_back(a); direction.push_back(b);
}

void Bspline::update(double time){
	curTime += time * (reverse ? -1 : 1);
	if(curTime >= duration || curTime <= 0) reverse = !reverse;
	if(curTime < 0) curTime = 0;
	if(curTime > duration) curTime = duration;

	double segDur = duration / segments.size();
	int segment = 0;
	while(segment * segDur <= curTime) segment++;
	segment--;

	double t = (curTime - segDur*segment)/segDur;
	glm::vec4 timeVec(pow(t, 3), pow(t, 2), t, 1);
	glm::vec3 dTimeVec(pow(t, 2), t, 1);

	glm::vec4 pos = timeVec*segments[segment];
	glm::vec3 rot = dTimeVec*dSegments[segment];



	glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(pos / pos[3]));
	glm::mat4 rotate = glm::lookAt(glm::vec3(0.0f), glm::normalize(glm::vec3(rot[0], rot[2], rot[1])), UP);
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f));

	if(!reverse){
		rotate = glm::rotate(rotate, glm::pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	rotate = glm::mat4(1.0);

	tNormal = rotate;
	tVert = translate * rotate * scale;
}
