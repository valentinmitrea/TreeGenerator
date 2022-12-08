#pragma once

#include "glm/glm.hpp"


class Vertex
{

public:
	float x, y, z;
	float normalX, normalY, normalZ;
	float textureX, textureY;

	Vertex(glm::vec3 position);
	Vertex(glm::vec3 position, glm::vec3 normal);
	Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texture);

};
