#include "tools/Vertex.h"


Vertex::Vertex(glm::vec3 position)
{
	this->x = position.x;
	this->y = position.y;
	this->z = position.z;
}


Vertex::Vertex(glm::vec3 position, glm::vec3 normal)
{
	this->x = position.x;
	this->y = position.y;
	this->z = position.z;

	this->normalX = normal.x;
	this->normalY = normal.y;
	this->normalZ = normal.z;
}


Vertex::Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 texture)
{
	this->x = position.x;
	this->y = position.y;
	this->z = position.z;

	this->normalX = normal.x;
	this->normalY = normal.y;
	this->normalZ = normal.z;

	this->textureX = texture.x;
	this->textureY = texture.y;
}
