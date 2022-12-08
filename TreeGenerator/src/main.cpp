#include "meshes/Ground.h"
#include "meshes/Tree.h"

#include "lab_shader_loader.hpp"
#include "lab_glut.hpp"
#include "lab_camera.hpp"
#include "lab_texture_loader.hpp"

#include <vector>
#include <ctime>


class Proiect : public lab::glut::WindowListener {

	int width = 800, height = 600;

	//matricea de modelare si proiectie, plus camera prin care privim scena
	glm::mat4 model_matrix, projection_matrix;
	float fovy = 90.0f, aspect = (float)width / (float)height, near_plane = 1.0f, far_plane = 200.0f;
	lab::Camera camera;

	Ground *ground;
	unsigned int ground_texture;
	std::vector<Tree*> trees;

	//pozitia luminii, matricile de vizualizare si proiectie ale luminii
	glm::vec3 light_position;
	glm::mat4 light_view_matrices[6], light_projection_matrix;

	int shadow_map_size = 1024;

	//textura cubemap si fbo-ul folosite de algoritmul Omnidirectional Shadow Map
	unsigned int depth_shader, shader;
	unsigned int depth_cubemap, depth_cubemap_fbo;

public:
	Proiect() {
		//setari pentru desenare, clear color seteaza culoarea de clear pentru ecran (format R,G,B,A)
		glClearColor(0.5, 0.5, 0.5, 1);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		model_matrix = glm::mat4(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
		projection_matrix = glm::perspective(fovy, aspect, near_plane, far_plane);
		camera.set(glm::vec3(0, 2, 0), glm::vec3(0, 2, -1), glm::vec3(0, 1, 0));

		//se contruieste o zona de ground pe care vor fi pusi copacii si se incarca textura pentru ground
		ground = new Ground(-50, 0, -50, 100, 100);
		ground_texture = lab::loadTextureBMP("resources/textures/ground.bmp");

		//se construieste o matrice de copaci
		for (int i = -30; i <= 30; i += 20)
			for (int j = -30; j <= 30; j += 20)
				trees.push_back(new Tree(i, 0, j, 5));

		//se pozitioneaza lumina punct in scena
		light_position = glm::vec3(-15, 15, -15);
		light_projection_matrix = glm::perspective(fovy, (float)shadow_map_size / (float)shadow_map_size, near_plane, far_plane);

		depth_shader = lab::loadShader("src/shaders/depth_vertex_shader.glsl", "src/shaders/depth_geometry_shader.glsl", "src/shaders/depth_fragment_shader.glsl");
		shader = lab::loadShader("src/shaders/vertex_shader.glsl", "src/shaders/fragment_shader.glsl");

		//se creeaza cubemap-ul pentru testul de adancime
		glGenTextures(1, &depth_cubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap);
		for (int i = 0; i < 6; i++)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, shadow_map_size, shadow_map_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		//se seteaza parametrii texturii cubemap
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		//se creeaza frame buffer object-ul in care se va randa scena din perspectiva luminii
		glGenFramebuffers(1, &depth_cubemap_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, depth_cubemap_fbo);

		//se ataseaza textura cubemap la frame buffer object-ul de adancimi
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_cubemap, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	~Proiect() {
		//stergere ground, textura ground si copaci
		delete ground;
		glDeleteTextures(1, &ground_texture);
		for (int i = 0; i < trees.size(); i++)
			delete trees[i];

		//distruge shadere
		glDeleteProgram(depth_shader);
		glDeleteProgram(shader);

		//distrugere textura cubemap si fbo
		glDeleteTextures(1, &depth_cubemap);
		glDeleteFramebuffers(1, &depth_cubemap_fbo);
	}


	//--------------------------------------------------------------------------------------------
	//functii de cadru ---------------------------------------------------------------------------

	//functie chemata inainte de a incepe cadrul de desenare, o folosim ca sa updatam situatia scenei (modelam/simulam scena)
	void notifyBeginFrame() {
		float angle = 0.005f;
		glm::vec3 lp = light_position;

		//rotatie lumina pe Oy
		light_position.x = lp.x * cos(angle) - lp.z * sin(angle);
		light_position.z = lp.x * sin(angle) + lp.z * cos(angle);

		//matricile de vizualizare ale luminii pentru fiecare directie: dreapta, stanga, sus, jos, fata, spate
		light_view_matrices[0] = glm::lookAt(light_position, light_position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
		light_view_matrices[1] = glm::lookAt(light_position, light_position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
		light_view_matrices[2] = glm::lookAt(light_position, light_position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
		light_view_matrices[3] = glm::lookAt(light_position, light_position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
		light_view_matrices[4] = glm::lookAt(light_position, light_position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
		light_view_matrices[5] = glm::lookAt(light_position, light_position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
	}

	//functia de afisare (lucram cu banda grafica)
	void notifyDisplayFrame() {
		//pas 1 -> se randeaza scena din perspectiva luminii
		glViewport(0, 0, shadow_map_size, shadow_map_size);

		//bindare frame buffer pentru testul de adancime
		glBindFramebuffer(GL_FRAMEBUFFER, depth_cubemap_fbo);
		glClearDepth(1.0);
		glClear(GL_DEPTH_BUFFER_BIT);

		//setare shadere si variabile folosite
		glUseProgram(depth_shader);
		glUniformMatrix4fv(glGetUniformLocation(depth_shader, "model_matrix"), 1, false, glm::value_ptr(model_matrix));
		for (int i = 0; i < 6; i++)
			glUniformMatrix4fv(glGetUniformLocation(depth_shader, ("light_view_matrices[" + std::to_string(i) + "]").c_str()), 1, false, glm::value_ptr(light_view_matrices[i]));
		glUniformMatrix4fv(glGetUniformLocation(depth_shader, "light_projection_matrix"), 1, false, glm::value_ptr(light_projection_matrix));
		glUniform3f(glGetUniformLocation(depth_shader, "light_position"), light_position.x, light_position.y, light_position.z);
		glUniform1f(glGetUniformLocation(depth_shader, "far_plane"), far_plane);

		//desenare ground si copaci
		ground->draw();
		for (int i = 0; i < trees.size(); i++)
			trees[i]->draw();


		//pas 2 -> se randeaza scena normal folosind depth cubemap-ul pentru a desena umbre
		glViewport(0, 0, width, height);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//setare shadere si variabile folosite
		glUseProgram(shader);
		glUniformMatrix4fv(glGetUniformLocation(shader, "model_matrix"), 1, false, glm::value_ptr(model_matrix));
		glUniformMatrix4fv(glGetUniformLocation(shader, "view_matrix"), 1, false, glm::value_ptr(camera.getViewMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(shader, "projection_matrix"), 1, false, glm::value_ptr(projection_matrix));
		glUniform3f(glGetUniformLocation(shader, "light_position"), light_position.x, light_position.y, light_position.z);
		glUniform3f(glGetUniformLocation(shader, "camera_position"), camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
		glUniform1f(glGetUniformLocation(shader, "far_plane"), far_plane);

		//activare si bindare texturi
		glActiveTexture(GL_TEXTURE0 + ground_texture);
		glBindTexture(GL_TEXTURE_2D, ground_texture);
		glUniform1i(glGetUniformLocation(shader, "ground_texture"), ground_texture);
		glActiveTexture(GL_TEXTURE0 + depth_cubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap);
		glUniform1i(glGetUniformLocation(shader, "depth_cubemap"), depth_cubemap);

		//desenare ground
		glUniform1i(glGetUniformLocation(shader, "element_to_draw"), 0);
		ground->draw();

		//desenare copaci
		glUniform1i(glGetUniformLocation(shader, "element_to_draw"), 1);
		for (int i = 0; i < trees.size(); i++)
			trees[i]->draw();
	}

	//functie chemata dupa ce am terminat cadrul de desenare (poate fi folosita pt modelare/simulare)
	void notifyEndFrame() {
	}

	//functie care e chemata cand se schimba dimensiunea ferestrei initiale
	void notifyReshape(int width, int height, int previous_width, int previous_height) {
		//reshape
		if (height == 0)
			height = 1;

		this->width = width;
		this->height = height;
		this->aspect = (float)width / (float)height;

		projection_matrix = glm::perspective(fovy, aspect, near_plane, far_plane);
	}


	//--------------------------------------------------------------------------------------------
	//functii de input output --------------------------------------------------------------------
	
	//tasta apasata
	void notifyKeyPressed(unsigned char key_pressed, int mouse_x, int mouse_y) {
		if(key_pressed == 27)
			lab::glut::close();
		if(key_pressed == 32)
		{
			//distruge shadere
			glDeleteProgram(depth_shader);
			glDeleteProgram(shader);

			depth_shader = lab::loadShader("src/shaders/depth_vertex_shader.glsl", "src/shaders/depth_geometry_shader.glsl", "src/shaders/depth_fragment_shader.glsl");
			shader = lab::loadShader("src/shaders/vertex_shader.glsl", "src/shaders/fragment_shader.glsl");
		}
		if(key_pressed == 't')
		{
			static bool wire = true;
			wire = !wire;
			glPolygonMode(GL_FRONT_AND_BACK, (wire ? GL_LINE : GL_FILL));
		}

		if (key_pressed == 'r')
			camera.set(glm::vec3(0, 2, 0), glm::vec3(0, 2, -1), glm::vec3(0, 1, 0));

		if (key_pressed == 'w')
			camera.translateForward(0.5f);
		if (key_pressed == 'a')
			camera.translateRight(-0.5f);
		if (key_pressed == 's')
			camera.translateForward(-0.5f);
		if (key_pressed == 'd')
			camera.translateRight(0.5f);

		if (key_pressed == 'i')
			camera.rotateFPSoX(-0.5f);
		if (key_pressed == 'j')
			camera.rotateFPSoY(-0.5f);
		if (key_pressed == 'k')
			camera.rotateFPSoX(0.5f);
		if (key_pressed == 'l')
			camera.rotateFPSoY(0.5f);

		if (key_pressed == 'q')
			camera.translateUpword(0.5f);
		if (key_pressed == 'z')
			camera.translateUpword(-0.5f);
	}

	//tasta ridicata
	void notifyKeyReleased(unsigned char key_released, int mouse_x, int mouse_y) {
	}
	//tasta speciala (up/down/F1/F2..) apasata
	void notifySpecialKeyPressed(int key_pressed, int mouse_x, int mouse_y) {
		if(key_pressed == GLUT_KEY_F1) lab::glut::enterFullscreen();
		if(key_pressed == GLUT_KEY_F2) lab::glut::exitFullscreen();
	}
	//tasta speciala ridicata
	void notifySpecialKeyReleased(int key_released, int mouse_x, int mouse_y) {
	}
	//drag cu mouse-ul
	void notifyMouseDrag(int mouse_x, int mouse_y) {
	}
	//am miscat mouseul (fara sa apas vreun buton)
	void notifyMouseMove(int mouse_x, int mouse_y) {
	}
	//am apasat pe un boton
	void notifyMouseClick(int button, int state, int mouse_x, int mouse_y) {
	}
	//scroll cu mouse-ul
	void notifyMouseScroll(int wheel, int direction, int mouse_x, int mouse_y) {
	}

};


int main() {
	//initializeaza GLUT (fereastra + input + context OpenGL)
	lab::glut::WindowInfo window(std::string("TreeGenerator"), 800, 600, 100, 100, true);
	lab::glut::ContextInfo context(3, 3, false);
	lab::glut::FramebufferInfo framebuffer(true, true, true, true);
	lab::glut::init(window, context, framebuffer);

	//initializeaza GLEW (ne incarca functiile openGL, altfel ar trebui sa facem asta manual!)
	glewExperimental = true;
	glewInit();
	std::cout << "GLEW: initializare" << std::endl;

	srand(time(NULL));

	//cream clasa noastra si o punem sa asculte evenimentele de la GLUT
	//DUPA GLEW!!! ca sa avem functiile de OpenGL incarcate inainte sa ii fie apelat constructorul (care creeaza obiecte OpenGL)
	Proiect proiect;
	lab::glut::setListener(&proiect);

	//taste
	std::cout << "Taste:" << std::endl;
	
	std::cout << "\tESC -> iesire" << std::endl;
	std::cout << "\tSPACE -> reincarca shadere" << std::endl;
	std::cout << "\tt -> toggle wireframe" << std::endl;
	std::cout << "\tr -> resetare camera" << std::endl << std::endl;

	std::cout << "\tw -> muta camera in fata" << std::endl;
	std::cout << "\ta -> muta camera stanga" << std::endl;
	std::cout << "\ts -> muta camera in spate" << std::endl;
	std::cout << "\td -> muta camera dreapta" << std::endl << std::endl;

	std::cout << "\ti -> rotire camera in sus" << std::endl;
	std::cout << "\tj -> rotire camera stanga" << std::endl;
	std::cout << "\tk -> rotire camera in jos" << std::endl;
	std::cout << "\tl -> rotire camera dreapta" << std::endl << std::endl;

	std::cout << "\tq -> deplasare camera in sus" << std::endl;
	std::cout << "\tz -> deplasare camera in jos" << std::endl << std::endl;

	//run
	lab::glut::run();

	return 0;
}
