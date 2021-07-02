#include <stdlib.h> // aleatorios
#include <time.h> // aleatorios

#include "files.h"
#include "model.h"
#include "cam.h"

#include <shader.h>
#include <texture.h>
 
// settings
const i32 SCR_WIDTH = 1280;
const i32 SCR_HEIGHT = 720;
const f32  ASPECT = (f32)SCR_WIDTH / (f32)SCR_HEIGHT;

// camera 
Cam* cam;

// timing
f32 deltaTime = 0.0f;
f32 lastFrame = 0.0f;
bool wireframe = false;

// lighting

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		deltaTime *= 10;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cam->processKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cam->processKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cam->processKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cam->processKeyboard(RIGHT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
		cam->processKeyboard(UP, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
		cam->processKeyboard(DOWN, deltaTime);
	}
}
void key_callback(GLFWwindow*, int key, int, int act, int) {
	wireframe ^= key == GLFW_KEY_E && act == GLFW_PRESS;
}
void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos) {
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		cam->movePov(xpos, ypos);
	}
	else {
		cam->stopPov();
	}	
}
void scroll_callback(GLFWwindow* window, f64 xoffset, f64 yoffset) {
	cam->processScroll((f32)yoffset);
}

f32 matrix[11][11];

/***********************************************/
f32 noise(f32 precision = 100.0f) {
	return (rand() % (int)precision + 1.0f) / precision;
}
i32 rescale(f32 num, f32 min, f32 max) {
	return (int)((max - min) * (num) / ((float)(1.0f)) + min);
}
void bilinear_interpolation(u32& n, u32 offset) {
	int xNearest, zNearest;
	float nearest, farest;
	//interpolamos valores en horizontal
	for (int x = 0; x < n; ++x) {
		if (x % offset == 0) {
			for (int z = 0; z < n - 1; ++z) {
				if (z % offset == 0) {
					zNearest = z;
					nearest = matrix[x][z];
					farest = matrix[x][z + offset];
					continue;
				}
				float point = z - zNearest;
				matrix[x][z] =
					(nearest * (offset - point) / (float)offset) +
					(farest * (point) / (float)offset);
			}
		}
	}
	//interpolamos valores en vertical
	for (int z = 0; z < n; ++z) {
		if (true) {
			for (int x = 0; x < n - 1; ++x) {
				if (x % offset == 0) {
					xNearest = x;
					nearest = matrix[x][z];
					farest = matrix[x + offset][z];
					continue;
				}
				float point = x - xNearest;
				matrix[x][z] =
					(nearest * (offset - point) / (float)offset) +
					(farest * (point) / (float)offset);
			}
		}
	}
}
void generate_random(u32& n, i32 min, i32 max, u32 offset) {
	//Solo generamos n�meros randoms, cada cierto intervalo
	//Aquellos que no se encuentren en el intervalo, quedan vacios
	float precision = 100.0f;
	for (int x = 0; x < n; ++x)
		for (int z = 0; z < n; ++z) {
			matrix[x][z] = 0.0f;
			if (x % offset == 0 && z % offset == 0)
				matrix[x][z] = noise();
		}

	bilinear_interpolation(n, offset);

	//Como este no es un espacio continuo, hay que truncar los valores
	for (int x = 0; x < n; ++x)
		for (int z = 0; z < n; ++z)
			matrix[x][z] = rescale(matrix[x][z], min, max);

	/*for (u32 x = 0; x < n; ++x) {
		for (u32 z = 0; z < n; ++z) {
			//std::cout << matrix[x][z] << "\t";
		}
		//std::cout << "\n";
	}
	for (u32 x = 0; x < n; ++x)
		for (u32 z = 0; z < n; ++z)
			matrix[x][z] = (int)(matrix[x][z]);
			*/
}
void load_positions(u32& n, std::vector<glm::vec3>& positions) {
	for (u32 x = 0; x < n; ++x) {
		for (u32 z = 0; z < n; ++z) {
			positions[x * n + z] = glm::vec3(x, matrix[x][z], z);
		}
	}
}
/***********************************************/


i32 main() {
	GLFWwindow* window = glutilInit(3, 3, SCR_WIDTH, SCR_HEIGHT, "Generacion procedural de terrenos");
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glEnable(GL_DEPTH_TEST);

	cam = new Cam();

	srand(time(NULL));
	u32 n = 11;
	std::vector<glm::vec3> positions(n * n);
	u32 amount = n * n;
	generate_random(n, -5, 15, 20);
	load_positions(n, positions);

	Files* files = new Files("D:/JC/UPC 2021-01/Comp. Graf/JackCruz/jack/jack",
		"D:/JC/UPC 2021-01/Comp. Graf/JackCruz/Git OpenGL/resources/textures",
		"D:/JC/UPC 2021-01/Comp. Graf/JackCruz/Git OpenGL/resources/objects");

	Shader* asteroidShader = new Shader("asteroids.vert", "asteroids.frag");
	Shader* planetShader = new Shader("planet.vert", "planet.frag");

	Model* planet = new Model(files, "alien/alien.obj");
	Model* rock = new Model(files, "dirt/dirt.obj");

	string textures_path = "D:/JC/UPC 2021-01/Comp. Graf/JackCruz/Git OpenGL/";
	tex::Texture* texture0 = new tex::Texture(GL_TEXTURE0, "agua.jpeg", textures_path + "resources/textures");
	tex::Texture* texture1 = new tex::Texture(GL_TEXTURE0, "dirt.jpeg", textures_path + "resources/textures");

	glm::vec3 lightPos = glm::vec3(1.0f);
	glm::vec3 lightColor = glm::vec3(1.0f);

	glm::mat4* modelMatrices;
	//modelMatrices = new glm::mat4[n*n];
	modelMatrices = new glm::mat4[amount];

	glm::mat4 modelaux;
	for (ui32 i = 0; i < n; i++) {
		for (ui32 j = 0; j < n; j++) {
			modelaux = glm::mat4(1.0f);
			modelaux = glm::translate(modelaux, positions[i*n+j]);
			modelMatrices[i*n+j] = modelaux;
		}
	}
	
	// configure instanced array
	// -------------------------

	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

	for (unsigned int i = 0; i < rock->meshes.size(); i++)
	{
		unsigned int VAO = rock->meshes[i].Vao;
		glBindVertexArray(VAO);
		// set attribute pointers for matrix (4 times vec4)
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}


	while (!glfwWindowShouldClose(window)) {
		// per-frame logic
		f32 currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		processInput(window);

		// render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);

		// planet
		planetShader->useProgram();
		lightPos.x = 2.0f * (cos(currentFrame) - sin(currentFrame));
		lightPos.z = 2.0f * (cos(currentFrame) + sin(currentFrame));

		planetShader->setVec3("lightPos", lightPos);
		planetShader->setVec3("lightColor", lightColor);
		planetShader->setVec3("viewPos", cam->pos);

		// view/projection transformations
		glm::mat4 projection = glm::perspective(cam->zoom, ASPECT, 0.1f, 100.0f);
		planetShader->setMat4("proj", projection);
		planetShader->setMat4("view", cam->getViewM4());

		// draw planet
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		planetShader->setMat4("model", model);
		texture1->Bind();
		planet->Draw(planetShader);

		// draw meteorites
		asteroidShader->useProgram();
		lightPos.x = 2.0f * (cos(currentFrame) - sin(currentFrame));
		lightPos.z = 2.0f * (cos(currentFrame) + sin(currentFrame));

		asteroidShader->setVec3("lightPos", lightPos);
		asteroidShader->setVec3("lightColor", lightColor);
		asteroidShader->setVec3("viewPos", cam->pos);

		// view/projection transformations
		projection = glm::perspective(cam->zoom, ASPECT, 0.1f, 100.0f);
		asteroidShader->setMat4("proj", projection);
		asteroidShader->setMat4("view", cam->getViewM4());

		// draw meteorites
		asteroidShader->useProgram();
		texture0->Bind();
		for (unsigned int i = 0; i < rock->meshes.size(); i++)
		{
			glBindVertexArray(rock->meshes[i].Vao);
			glDrawElementsInstanced(GL_TRIANGLES, rock->meshes[i].indices.size(),
				GL_UNSIGNED_INT, 0, amount);
			glBindVertexArray(0);
		}

		cout << rock->meshes.size() << endl;
		// glfw: swap buffers and poll IO event (keys pressed/realesed, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	// de-allocate all resources once they've outlived their purpose
	delete cam;
	delete planetShader;
	delete asteroidShader;
	delete rock;
	delete planet;

	return 0;
}