#pragma once

#include "tools/Vertex.h"

#include <vector>


class Ground
{

	unsigned int vao, vbo, ibo, index = 0;
	std::vector<Vertex> vertices;
	std::vector<glm::uvec3> indices;

public:
	Ground(float x, float y, float z, int sizeX, int sizeZ);
	~Ground();

	void draw();

};
