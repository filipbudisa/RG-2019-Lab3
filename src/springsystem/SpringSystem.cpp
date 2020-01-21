#include "SpringSystem.h"
#include "../Game.h"
#include "../storage/Storage.h"
#include "../Game.h"

SpringSystem::SpringSystem(WorldObject *object, unsigned n, float mass) : object(object), n(n){
	Storage::sSystems.push_back(this);
	PhysicsEngine::physComps.push_back(this);

	constructPoints(mass / pow(n, 2));
	constructSprings();
}

void SpringSystem::constructPoints(float mass){
	points.resize(pow(n, 2));
	glPoints.resize(pow(n, 2));
	mesh.resize(n);

	for(int i = 0; i < points.size(); i++){
		points[i] = new MassPoint(&(object->renderComponent->mesh.vertices[i]), &object->renderComponent->transforms, mass);
		glPoints[i] = { { 0, 0, 0 }, { 0, 0, 0 } };

		int j = i / n;
		mesh[j].push_back(points[i]);
	}
}

void SpringSystem::constructSprings(){
	int k1 = 50000;
	int k2 = 500000;
	int k3 = 500;

	for(int i = 0; i < mesh.size(); i++){
		for(int j = 0; j < mesh[i].size(); j++){
			if(i > 0){
				//Spring* a = new Spring(points[i][j], points[i-1][j], k1);
				Spring* a = new Spring(i * mesh.size() + j, (i-1) * mesh.size() + j, k1);
				addSpring(a);
			}

			if(j > 0){
				//Spring* b = new Spring(points[i][j], points[i][j-1], k1);
				Spring* b = new Spring(i * mesh.size() + j, i * mesh.size() + j - 1, k1);
				addSpring(b);
			}

			/*if(j > 0 && i > 0){
				Spring* b = new Spring((i-1) * mesh.size() + j, i * mesh.size() + j - 1, k2);
				addSpring(b);
			}

			if(j > 0 && i < mesh.size()-1){
				Spring* b = new Spring((i+1) * mesh.size() + j, i * mesh.size() + j - 1, k2);
				addSpring(b);
			}


			if(i > 1){
				//Spring* a = new Spring(points[i][j], points[i-1][j], k1);
				Spring* a = new Spring(i * mesh.size() + j, (i-2) * mesh.size() + j, k3);
				addSpring(a);
			}

			if(j > 1){
				//Spring* b = new Spring(points[i][j], points[i][j-1], k1);
				Spring* b = new Spring(i * mesh.size() + j, i * mesh.size() + j - 2, k3);
				addSpring(b);
			}*/
		}
	}
}

void SpringSystem::resetForce(){
	for(int i = 0; i < points.size(); i++){
		points[i]->resetForce();
		glPoints[i].force = { 0, 0, 0 };
	}
}

void SpringSystem::collide(CollisionComponent *collidor){
	return;

	for(MassPoint* point : points){
		glm::vec3 pos =  point->getPosition() * object->scale + object->position;
		glm::vec3 diff = collidor->collide(pos);
		//point->move(diff);
		point->addVelocity(diff * 500.0f);
	}
}

void SpringSystem::update(double time){

	for(Spring* spring : springs){
		spring->update(time);
	}

	return;

	for(MassPoint* point : points){
		point->update(time);
	}

	if(drawMesh){
		for(Spring *spring : springs){
			spring->updateVertices();
		}
	}

	/*int v = 0;
	for(int i = 1; i < mesh.size(); i++){
		for(int j = 0; j < mesh.size()-1; j++){
			glm::vec3 a = mesh[i][j]->getPosition();
			glm::vec3 b = mesh[i-1][j]->getPosition();
			glm::vec3 c = mesh[i][j+1]->getPosition();
			glm::vec3 normal = glm::cross(b - a, c - a);
			normal = glm::normalize(normal);
			vertices[v++] = { a, { 1.0, 1.0, 1.0 }, normal };
			vertices[v++] = { b, { 1.0, 1.0, 1.0 }, normal };
			vertices[v++] = { c, { 1.0, 1.0, 1.0 }, normal };

			a = mesh[i-1][j]->getPosition();
			b = mesh[i-1][j+1]->getPosition();
			c = mesh[i][j+1]->getPosition();
			normal = glm::cross(b - c, b - a);
			normal = glm::normalize(normal);
			vertices[v++] = { a, { 1.0, 1.0, 1.0 }, normal };
			vertices[v++] = { b, { 1.0, 1.0, 1.0 }, normal };
			vertices[v++] = { c, { 1.0, 1.0, 1.0 }, normal };
		}
	}*/
}

void SpringSystem::addSpring(Spring* spring){
	Storage::springs.push_back(spring);
	springs.push_back(spring);
	spring->setSystem(this);

	std::pair<uint16_t, uint16_t> indexes = spring->getIndexes();
	glSprings.push_back({ indexes.first, indexes.second, spring->k, spring->length });
}

void SpringSystem::addPoint(MassPoint* point){
	points.push_back(point);
}

void SpringSystem::setMesh(const std::vector<std::vector<MassPoint *>>& mesh){
	SpringSystem::mesh = mesh;
}

const std::vector<std::vector<MassPoint *>>& SpringSystem::getMesh() const{
	return mesh;
}

MassPoint *SpringSystem::getPoint(int n){
	return points[n];
}

int SpringSystem::getNoPoints(){
	return points.size();
}

void SpringSystem::setFixed(int i, int j, bool fixed){
	points[i * n + j]->setFixed(fixed);
	glPoints[i * n + j].fixed = fixed;
}

void SpringSystem::cleanup(){
	for(MassPoint* mp : points){
		delete mp;
	}

	for(Spring* mp : springs){
		delete mp;
	}
}
