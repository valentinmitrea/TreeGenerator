#pragma once

#include "tools/Vertex.h"

#include <vector>

#define MAX_LEVEL 4
#define INITIAL_WIDTH 0.4f
#define INITIAL_NUMBER_OF_EXTENTIONS 4


class Tree
{

	unsigned int vao, vbo, ibo, index = 0;
	std::vector<Vertex> vertices;
	std::vector<glm::uvec3> indices;

public:
	Tree(float x, float y, float z, float height);
	~Tree();

private:
	glm::vec3 rotate(glm::vec3 point, float angle, glm::vec3 axis, glm::vec3 center);

	std::vector<glm::vec3> generateConeBase(glm::vec3 axis, glm::vec3 center, float width);
	void generateBranch(int level, glm::vec3 start, glm::vec3 end, float width, int nrOfExtensions);

public:
	void draw();

};
