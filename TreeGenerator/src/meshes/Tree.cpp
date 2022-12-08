#include "meshes/Tree.h"
#include "glew/glew.h"
#include "tools/Random.h"


Tree::Tree(float x, float y, float z, float height)
{
	//se genereaza trunchiul principal si, apoi, recursiv ramurile copacului
	generateBranch(0, glm::vec3(x, y, z), glm::vec3(x, y + height, z), INITIAL_WIDTH, INITIAL_NUMBER_OF_EXTENTIONS);

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

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(glm::uvec3), &indices[0], GL_STATIC_DRAW);
}


Tree::~Tree()
{
	glDeleteBuffers(1, &vao);
	glDeleteVertexArrays(1, &vbo);
	glDeleteBuffers(1, &ibo);
}


//functie ce realizeaza rotatia cu unghiul angle a punctului primit ca parametru dupa axa axis in jurul punctului centru
glm::vec3 Tree::rotate(glm::vec3 point, float angle, glm::vec3 axis, glm::vec3 center)
{
	glm::mat3 rotationMatrix;

	//se va construi matricea de rotatie pentru rotatie cu unghiul angle in jurul axei axis
	rotationMatrix[0][0] = axis.x * axis.x * (1 - cos(angle)) + cos(angle);
	rotationMatrix[0][1] = axis.x * axis.y * (1 - cos(angle)) + axis.z * sin(angle);
	rotationMatrix[0][2] = axis.x * axis.z * (1 - cos(angle)) - axis.y * sin(angle);
	rotationMatrix[1][0] = axis.y * axis.x * (1 - cos(angle)) - axis.z * sin(angle);
	rotationMatrix[1][1] = axis.y * axis.y * (1 - cos(angle)) + cos(angle);
	rotationMatrix[1][2] = axis.y * axis.z * (1 - cos(angle)) + axis.x * sin(angle);
	rotationMatrix[2][0] = axis.z * axis.x * (1 - cos(angle)) + axis.y * sin(angle);
	rotationMatrix[2][1] = axis.z * axis.y * (1 - cos(angle)) - axis.x * sin(angle);
	rotationMatrix[2][2] = axis.z * axis.z * (1 - cos(angle)) + cos(angle);

	return rotationMatrix * (point - center) + center;
}


//functie ce genereaza o baza de con (ce reprezinta un cerc)
std::vector<glm::vec3> Tree::generateConeBase(glm::vec3 axis, glm::vec3 center, float width)
{
	std::vector<glm::vec3> points(10);

	//se determina punctele de pe cercul cu centrul in center si raza width
	//ne dorim ca axa sa fie perpendiculara pe acest cerc
	for (int i = 0; i < 10; i++)
		if (i == 0)
			points[i] = glm::vec3(-axis.y, axis.x, 0) * width + center;
		else
			points[i] = rotate(points[0], i * glm::radians(360.0f) / 10, axis, center);

	//toate varfurile bazei conului vor avea aceeasi normala (perpendiculara pe unul dintre triunghiurile din care este formata baza)
	glm::vec3 normal = glm::cross(points[1] - points[0], center - points[0]);

	//construim triunghiuri intre punctul din centru si punctele de pe cerc
	for (int i = 0; i <= 10; i++)
		if (i == 0)
			vertices.push_back(Vertex(center, normal));
		else
		{
			vertices.push_back(Vertex(points[i - 1], normal));
			indices.push_back(glm::uvec3(index, index + i, index + (i != 10 ? i + 1 : 1)));
		}

	//updatam valoarea index-ului de referinta
	index += 11;

	return points;
}


//functie ce genereaza o ramura la nivelul level determinata de informatiile primite ca parametrii
void Tree::generateBranch(int level, glm::vec3 start, glm::vec3 end, float width, int nrOfExtensions)
{
	if (level == MAX_LEVEL)
		return;

	//calculam in axis directia ce o are ramura curenta
	glm::vec3 axis = glm::normalize(end - start);

	//construim bazele unui trunchi de con pentru ramura curenta
	std::vector<glm::vec3> pointsDown = generateConeBase(axis, start, width);
	std::vector<glm::vec3> pointsTop = generateConeBase(axis, end, width / 2);

	//pornind de la bazele trunchiului de con, vom genera lateralul conului format din triunghiuri
	//trebuie duplicati vertecsii, deoarece aceste triunghiuri au alte normale fata de cei de pe baze
	for (int i = 0; i < 10; i++)
	{
		glm::vec3 normal = glm::cross(pointsDown[i != 9 ? i + 1 : 0] - pointsDown[i], pointsTop[i] - pointsDown[i]);
		int offset = i != 9 ? 2 : -2 * i;

		vertices.push_back(Vertex(pointsDown[i], normal));
		vertices.push_back(Vertex(pointsTop[i], normal));

		//construim un quad pe lateralul conului
		indices.push_back(glm::uvec3(index, index + 1, index + offset));
		indices.push_back(glm::uvec3(index + 1, index + offset, index + offset + 1));
		index += 2;
	}

	//se genereaza ramuri extensii pentru ramura curenta
	for (int i = 1; i <= nrOfExtensions; i++)
	{
		//calculam pozitia de start a noii ramuri ca fiind o pozitie random intre start-ul si end-ul ramurii curente
		//ramurile ce pleaca din trunchiul principal vor fi puse mai aproape de partea de sus a trunchiului (altfel copacul ar avea o forma ciudata)
		int jointIndex = (level == 0) ? Random::randomInt(10, 20) : Random::randomInt(0, 20);
		float positionX = start.x + jointIndex * (end.x - start.x) / 20;
		float positionY = start.y + jointIndex * (end.y - start.y) / 20;
		float positionZ = start.z + jointIndex * (end.z - start.z) / 20;

		//pozitia finala a noii ramuri se va determina prin deplasarea random pe Ox si Oz a punctului de start calculat mai sus
		float offsetY = (end.y - start.y) / 2;
		float offsetX = (Random::randomInt(0, 1) == 0) ? Random::randomFloat(-offsetY, -offsetY / 2) : Random::randomFloat(offsetY / 2, offsetY);
		float offsetZ = (Random::randomInt(0, 1) == 0) ? Random::randomFloat(-offsetY, -offsetY / 2) : Random::randomFloat(offsetY / 2, offsetY);

		//vom apela recursiv functia pentru a construi noi ramuri
		generateBranch(level + 1, glm::vec3(positionX, positionY, positionZ), glm::vec3(positionX + offsetX, positionY + offsetY, positionZ + offsetZ), width / 2, nrOfExtensions + 2);
	}
}


void Tree::draw()
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 3 * indices.size(), GL_UNSIGNED_INT, 0);
}
