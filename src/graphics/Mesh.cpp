#include <sstream>
#include "Mesh.h"

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices) : vertices(vertices), indices(indices){ }

Mesh Mesh::generateSphere(float radius, int sectorCount, int stackCount){
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	float x, y, z, xy;                              // vertex position

	float sectorStep = 2.0 * M_PI / sectorCount;
	float stackStep = M_PI / stackCount;
	float sectorAngle, stackAngle;

	for(int i = 0; i <= stackCount; ++i)
	{
		stackAngle = M_PI / 2.0 - i * stackStep;        // starting from pi/2 to -pi/2
		xy = radius * cosf(stackAngle);             // r * cos(u)
		z = radius * sinf(stackAngle);              // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for(int j = 0; j <= sectorCount; ++j)
		{
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

			// vertex position (x, y, z)
			x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
			y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
			vertices.push_back({{ x, y, z }, { 1.0f, 1.0f, 1.0f }});
		}
	}

	int k1, k2;
	for(int i = 0; i < stackCount; ++i)
	{
		k1 = i * (sectorCount + 1);     // beginning of current stack
		k2 = k1 + sectorCount + 1;      // beginning of next stack

		for(int j = 0; j < sectorCount; ++j, ++k1, ++k2)
		{
			// 2 triangles per sector excluding first and last stacks
			// k1 => k2 => k1+1
			if(i != 0)
			{
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			// k1+1 => k2 => k2+1
			if(i != (stackCount-1))
			{
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
	}

	return Mesh(vertices, indices);
}

Mesh Mesh::generatePlane(glm::vec2 start, glm::vec2 end, int n){
	return generatePlane(start, end, n, false);
}

Mesh Mesh::generatePlane(glm::vec2 start, glm::vec2 end, int n, bool alt){

	glm::vec2 inc = (end - start) / (float) n;

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	for(int i = 0; i < n; i++){
		for(int j = 0; j < n; j++){
			glm::vec2 pos = start + inc * glm::vec2({ i, j });

			vertices.push_back({{pos.x, alt ? pos.y : 0, alt ? 0 : pos.y},
								{1,     1, 1},
								{0,     1, 0}});
		}
	}

	for(int k = n; k < pow(n, 2); k++){
		int i = k / n;
		int j = k % n;

		if(j == n-1) continue;

		indices.push_back(i * n + j);
		indices.push_back((i-1) * n + j);
		indices.push_back(i * n + j + 1);

		indices.push_back((i-1) * n + j);
		indices.push_back((i-1) * n + j + 1);
		indices.push_back(i * n + j + 1);
	}

	return { vertices, indices };
}

Mesh Mesh::load(const std::string &filename){
	std::ifstream file(filename);

	if(!file.is_open()){
		std::cout << filename << std::endl;
		throw std::runtime_error("failed to open file!");
	}

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	std::string line;
	std::stringstream ss;

	while(getline(file, line)){
		if(line[0] == '#') continue;

		char *buffer = strtok(line.data(), " ");
		if(*buffer == 'v'){
			glm::vec3 pos;

			pos.x = std::stod(strtok(nullptr, " "));
			pos.y = std::stod(strtok(nullptr, " "));
			pos.z = std::stod(strtok(nullptr, " "));

			vertices.push_back({ pos, { 0.5f, 0.5f, 0.5f }});
		}else if(*buffer == 'f'){
			indices.push_back(std::stoi(strtok(nullptr, " ")) - 1);
			indices.push_back(std::stoi(strtok(nullptr, " ")) - 1);
			indices.push_back(std::stoi(strtok(nullptr, " ")) - 1);
		}
	}

	file.close();

	return Mesh(vertices, indices);
}