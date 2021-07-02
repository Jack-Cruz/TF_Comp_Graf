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
vec3 lightPos(1.2f, 1.0f, 2.0f);

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

	Shader* lightingShader = new Shader("lighting_casters.vert", "lighting_casters.frag");
	Shader* lightCubeShader = new Shader("light_cube.vert", "light_cube.frag");

	Cube* cube = new Cube(0.9, 0.9, 0.9);

	// positions all containers
	vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	// positions all point lights
	vec3 pointLightPositions[4] = {
		vec3( 0.7f,  0.2f,   2.0f),
		vec3( 2.3f, -3.3f,  -4.0f),
		vec3(-4.0f,  2.0f, -12.0f),
		vec3( 0.0f,  0.0f,  -3.0f)
	};

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

	// shader configuration
	lightingShader->useProgram();
	lightingShader->setI32("material.diffuse", 0);
	lightingShader->setI32("material.specular", 1);


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

		// be sure to activate shader when setting uniforms/drawing objects
		lightingShader->useProgram();
		lightingShader->setVec3("viewPos", cam.getPos());
		lightingShader->setF32("material.shininess", 32.0f);

		/*  
			Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index
			the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
			by defining light types as classes and set their value in there, or by using a more efficient uniform approach
			by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
		*/
		// directional light
		lightingShader->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		lightingShader->setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
		lightingShader->setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
		lightingShader->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
		
		// point light 1
		lightingShader->setVec3("pointLights[0].position", pointLightPositions[0]);
		lightingShader->setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader->setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
		lightingShader->setF32("pointLights[0].constant", 1.0f); // attenuation (distance = 50)
		lightingShader->setF32("pointLights[0].linear", 0.09f);
		lightingShader->setF32("pointLights[0].quadratic", 0.032f);

		// point light 2
		//lightingShader->setVec3("pointLights[1].position", 2.3f, -3.3f, -4.0f);
		lightingShader->setVec3("pointLights[1].position", pointLightPositions[1]);
		lightingShader->setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader->setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader->setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
		lightingShader->setF32("pointLights[1].constant", 1.0f);
		lightingShader->setF32("pointLights[1].linear", 0.09f);
		lightingShader->setF32("pointLights[1].quadratic", 0.032f);

		// point light 3 
		//lightingShader->setVec3("pointLights[2].position", -4.0f, 2.0f, -12.0f);
		lightingShader->setVec3("pointLights[2].position", pointLightPositions[2]);
		lightingShader->setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader->setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader->setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
		lightingShader->setF32("pointLights[2].constant", 1.0f);
		lightingShader->setF32("pointLights[2].linear", 0.09f);
		lightingShader->setF32("pointLights[2].quadratic", 0.032f);

		// point light 4
		//lightingShader->setVec3("pointLights[3].position", 0.0f, 0.0f, -3.0f);
		lightingShader->setVec3("pointLights[3].position", pointLightPositions[3]);
		lightingShader->setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
		lightingShader->setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
		lightingShader->setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
		lightingShader->setF32("pointLights[3].constant", 1.0f);
		lightingShader->setF32("pointLights[3].linear", 0.09f);
		lightingShader->setF32("pointLights[3].quadratic", 0.032f);

		// spotLight
		lightingShader->setVec3("spotLight.position", cam.getPos());
		lightingShader->setVec3("spotLight.direction", cam.getFront());
		lightingShader->setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		lightingShader->setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		lightingShader->setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		lightingShader->setF32("spotLight.constant", 1.0f);
		lightingShader->setF32("spotLight.linear", 0.09f);
		lightingShader->setF32("spotLight.quadratic", 0.032f);
		lightingShader->setF32("spotLight.cutOff", cos(radians(12.5f)));
		lightingShader->setF32("spotLight.outerCutOff", cos(radians(15.0f)));
		
		// view/projection transformations
		mat4 projection = perspective(cam.getZoom(), ASPECT, 0.1f, 100.0f);
		lightingShader->setMat4("proj", projection);
		lightingShader->setMat4("view", cam.getViewM4());
		
		// world transformation
		mat4 model = mat4(1.0f);
		lightingShader->setMat4("model", model);

		// bind diffuse map
		diffuseMap->Bind();

		// bind specular map
		specularMap->Bind();

		// render the cubes
		cubeVao->Bind();
		for (ui32 i = 0; i < 10; i++) {
			// calculate the model matrix for each object and pass it to shader before drawing
			mat4 model = mat4(1.0f);
			model = translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = rotate(model, radians(angle), vec3(1.0f, 0.3f, 0.5f));
			lightingShader->setMat4("model", model);

			glDrawElements(GL_TRIANGLES, cube->getISize(), GL_UNSIGNED_INT, 0);
		}


		// also draw the lamp object(s)
		lightCubeShader->useProgram();
		lightCubeShader->setMat4("proj", projection);
		lightCubeShader->setMat4("view", cam.getViewM4());

		// light color
		lightCubeShader->setVec3("lightColor", vec3(1.0f));
		
		// we now draw as many bulbs as we have point lights
		lightCubeVao->Bind();
		for (ui32 i = 0; i < 4; i++) {
			mat4 model = mat4(1.0f);
			model = translate(model, pointLightPositions[i]);
			model = scale(model, vec3(0.2f)); // a smaller cube
			lightCubeShader->setMat4("model", model);

			glDrawElements(GL_TRIANGLES, cube->getISize(), GL_UNSIGNED_INT, 0);

		}


		// glfw: swap buffers and poll IO event (keys pressed/realesed, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	}

	// de-allocate all resources once they've outlived their purpose
	delete cubeVao;
	delete lightCubeVao;
	delete vbo;
	delete ebo;

	delete cube;
	delete lightingShader;
	delete lightCubeShader;

	delete diffuseMap;
	delete specularMap;

	return 0;
}