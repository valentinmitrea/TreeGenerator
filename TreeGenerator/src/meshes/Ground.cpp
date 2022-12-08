#include "meshes/Ground.h"
#include "glew/glew.h"


Ground::Ground(float x, float y, float z, int sizeX, int sizeZ)
{
	//vom construi ground-ul ca fiind o matrice de patrate
	for (int i = x; i < x + sizeX; i += 5)
		for (int j = z; j < z + sizeZ; j += 5)
		{
			//normalele pentru ground sunt vectori aflati de-a lungul axei Oy
			glm::vec3 normal = glm::vec3(0, 1, 0);

			vertices.push_back(Vertex(glm::vec3(i, y, j), normal, glm::vec2(0, 0)));
			vertices.push_back(Vertex(glm::vec3(i + 5, y, j), normal, glm::vec2(0, 1)));
			vertices.push_back(Vertex(glm::vec3(i, y, j + 5), normal, glm::vec2(1, 0)));
			vertices.push_back(Vertex(glm::vec3(i + 5, y, j + 5), normal, glm::vec2(1, 1)));

			indices.push_back(glm::uvec3(index, index + 1, index + 2));
			indices.push_back(glm::uvec3(index + 3, index + 1, index + 2));
			index += 4;
		}

	//generarea si bindarea vao, vbo, ibo
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(glm::uvec3), &indices[0], GL_STATIC_DRAW);
}


Ground::~Ground()
{
	glDeleteBuffers(1, &vao);
	glDeleteVertexArrays(1, &vbo); 
	glDeleteBuffers(1, &ibo);
}


void Ground::draw()
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 3 * indices.size(), GL_UNSIGNED_INT, 0);
}
