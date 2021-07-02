#include <vector>
#include <stdlib.h> // aleatorios
#include <time.h> // aleatorios
#include <math.h>

#include "vao.h"
#include "vbo.h"
#include "ebo.h"
#include "shader.h"
#include "cube.h"
#include "texture.h"
#include "camera.h"

// settings
const i32 SCR_WIDTH = 960;
const i32 SCR_HEIGHT = 540;
const f32  ASPECT = 16.0f / 9.0f;

// camera
Cam cam(0.0f, 0.0f, 5.0f);
f32 lastx;
f32 lasty;
bool firstMouse = true;

// timing
f32 deltaTime = 0.0f;
f32 lastFrame = 0.0f;

// lighting
vec3 lightPos(1.2f, 1.0f, 0.0f);

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		deltaTime *= 10;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cam.processKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cam.processKeyboard(BACKWARD, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) 
		cam.processKeyboard(LEFT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) 
		cam.processKeyboard(RIGTH, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) 
		cam.processKeyboard(UP, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) 
		cam.processKeyboard(DOWN, deltaTime);

}
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastx = xpos;
		lasty = ypos;
		firstMouse = false;
	}
	f32 xoffset = xpos - lastx;
	f32 yoffset = lasty - ypos;

	cam.processMouse(xoffset, yoffset);
	lastx = xpos;
	lasty = ypos;
	
}
void scroll_callback(GLFWwindow* window, f64 xoffset, f64 yoffset) {
	cam.processScroll((f32)yoffset);
}

i32 main() {
	GLFWwindow* window = glutilInit(3, 3, SCR_WIDTH, SCR_HEIGHT, "Generacion procedural de terrenos");
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	Shader* lightingShader = new Shader("lighting_maps.vert", "lighting_maps.frag");
	Shader* lightCubeShader = new Shader("light_cube.vert", "light_cube.frag");

	Cube* cube = new Cube(0.9, 0.9, 0.9);
	
	Vao* cubeVao = new Vao();
	Vbo* vbo = new Vbo();
	Ebo* ebo = new Ebo();

	vbo->SetBufferData(cube->getVSize(), cube->getVertices());
	ebo->SetBufferData(cube->getISize(), cube->getIndices());

	cubeVao->SetVertexAttributes(0, 3, 11 * FSIZE, 0); // posicion
	cubeVao->SetVertexAttributes(1, 3, 11 * FSIZE, 3); // normal
	cubeVao->SetVertexAttributes(2, 2, 11 * FSIZE, 9); // texCoords
	cubeVao->Unbind();

	Vao* lightCubeVao = new Vao();
	vbo->Bind();
	ebo->Bind();
	lightCubeVao->SetVertexAttributes(0, 3, 11 * FSIZE, 0); // posicion
	lightCubeVao->Unbind();


	glEnable(GL_DEPTH_TEST);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	

	Texture* diffuseMap = new Texture(GL_TEXTURE0, "container2.png");
	Texture* specularMap = new Texture(GL_TEXTURE1, "container2_specular.png");

	lightingShader->useProgram();
	lightingShader->setI32("material.diffuse", 0);
	lightingShader->setI32("material.specular", 1);



	while (!glfwWindowShouldClose(window)) {
		f32 currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;


		processInput(window);
		glClearColor(0.1f, 0.1f, 0.1f, 1.1f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float theta = glfwGetTime();

		// ====================================================================
		// Cubo de material
		lightingShader->useProgram();
		
		// Posiciones luz y cámara (luz posicion relativa)
		
		mat4 lmodel = mat4(1.0f);
		lmodel = rotate(lmodel, theta, { 0.0, 1.0, 0.0 });
		lmodel = translate(lmodel, lightPos);
		lmodel = scale(lmodel, vec3(0.2f)); // a smaller cube
		vec3 rel = vec3(lmodel * vec4(lightPos, 1.0));
		
		lightingShader->setVec3("light.position", rel); // xyzLht
		lightingShader->setVec3("viewPos", cam.getPos()); // xyz


		// light properties
		lightingShader->setVec3("light.ambient", 0.5f, 0.5f, 0.5f);
		lightingShader->setVec3("light.diffuse", 0.5, 0.5f, 0.5f);
		lightingShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);

		// material properties
		lightingShader->setVec3("material.specular", 0.5f, 0.5f, 0.5f); //xyzMat
		lightingShader->setF32("material.shininess", 32.0f); //xyzMat


		// view/projection transformations
		mat4 projection = perspective(cam.getZoom(), ASPECT, 0.1f, 100.0f);
		lightingShader->setMat4("proj", projection);
		lightingShader->setMat4("view", cam.getViewM4());
		
		// world transformation
		mat4 model = mat4(1.0f);
		lightingShader->setMat4("model", model);
		
		// bind diffuse and specular map
		diffuseMap->Bind();
		specularMap->Bind();

		// draw
		cubeVao->Bind();
		glDrawElements(GL_TRIANGLES, cube->getISize(), GL_UNSIGNED_INT, 0);


		// ====================================================================
		// lamp object
		lightCubeShader->useProgram();

		// light color
		vec3 lightColor = vec3(1.0f);
		lightCubeShader->setVec3("lightColor", lightColor);

		// transformations
		lightCubeShader->setMat4("proj", projection);
		lightCubeShader->setMat4("view", cam.getViewM4());
		lightCubeShader->setMat4("model", lmodel);

		// draw
		lightCubeVao->Bind();
		glDrawElements(GL_TRIANGLES, cube->getISize(), GL_UNSIGNED_INT, 0);


		glfwSwapBuffers(window);
		glfwPollEvents();
		
	}
	
	delete lightCubeVao;
	delete cubeVao;
	delete vbo;
	delete ebo;

	delete cube;
	delete lightingShader;
	delete lightCubeShader;

	delete diffuseMap;
	delete specularMap;


	return 0;
}