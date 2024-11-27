#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#include <iostream>
#include <chrono>
#include <thread>

const int SCR_WIDTH = 1920;
const int SCR_HEIGHT = 1080;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
glm::vec3 rgb(int r, int g, int b);
std::vector<float> generateGridVertices(int gridSize, float spacing);

// camera
Camera camera(glm::vec3(0.0f, 1.0, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// direction *from* the lightsource
static glm::vec3 lightDirection = glm::vec3(-1.0, -2.0, -1.0);

bool mouseFree = false;
bool showDebug = true;

glm::vec3 cubeColor = rgb(36, 176, 181);
glm::vec3 planeColor = rgb(100, 100, 100);
glm::vec3 gridColor = rgb(255, 255, 255);

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main(int argc, char* argv[]) {
	glfwInit();

	// Create window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Application", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		return -1;
	}
	glfwMakeContextCurrent(window);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	glEnable(GL_DEPTH_TEST);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	Shader mainShader("vert.glsl", "frag.glsl");
	Shader lightShader("lightVert.glsl", "lightFrag.glsl");
	Shader gridShader("gridVert.glsl", "gridFrag.glsl");

	Model mainObj("suzanne_smooth.obj");
	Model planeObj("plane.obj");

	unsigned int gridVAO, gridVBO;
	int gridSize = 100; // Number of lines in each direction
	float spacing = 1.0f; // Spacing between lines

	std::vector<float> gridVertices = generateGridVertices(gridSize, spacing);

	glGenVertexArrays(1, &gridVAO);
	glGenBuffers(1, &gridVBO);

	glBindVertexArray(gridVAO);

	glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
	glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	while (!glfwWindowShouldClose(window)) {
		// per-frame time logic
		// --------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glClearColor(0.2f, 0.2f, 0.2f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mainShader.use();

		mainShader.setVec3("lightDirection", lightDirection);

		// pass projection matrix to shader (note that in this case it could change every frame)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		// camera/view transformation
		glm::mat4 view = camera.GetViewMatrix();
		glm::vec3 viewPos = camera.Position;
		mainShader.setMat4("view", view);
		mainShader.setVec3("viewPos", viewPos);

		// pass transformation matrices to the shader
		mainShader.setMat4("projection", projection); // note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
		
		mainShader.setFloat("iTime", (float)glfwGetTime());

		// Draw cube
		glm::mat4 model = glm::mat4(1.0f);
		//model = glm::scale(model, glm::vec3(0.001f, 0.001f, 0.001f));
		model = glm::translate(model, glm::vec3(0.0, 2.0, 0.0));
		model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0, 1.0, 0.0));
		model = glm::rotate(model,  glm::pi<float>() / -5, glm::vec3(1.0, 0.0, 0.0));
		mainShader.setMat4("model", model);
		mainShader.setVec3("baseColor", cubeColor);
		mainObj.draw();

		// Draw plane
		mainShader.setVec3("baseColor", planeColor);
		model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(10.0, 10.0, 10.0));
		mainShader.setMat4("model", model);
		planeObj.draw();

		gridShader.use();

		glm::vec4 gridColor(0.5f, 0.5f, 0.5f, 0.5f); // Semi-transparent gray
		gridShader.setVec4("gridColor", gridColor);

		glm::mat4 gridModel = glm::mat4(1.0f); // Position the grid at the origin
		gridShader.setMat4("model", gridModel);
		gridShader.setMat4("view", view);
		gridShader.setMat4("projection", projection);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindVertexArray(gridVAO);
		glDrawArrays(GL_LINES, 0, gridVertices.size() / 3);
		glBindVertexArray(0);

		glDisable(GL_BLEND);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
		showDebug = !showDebug;
	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
		if (!mouseFree) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			mouseFree = true;
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			mouseFree = false;
			firstMouse = true; // Reset to avoid sudden jumps in camera
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

}


void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	if (mouseFree) return;

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	// Only process camera input if ImGui does not want the mouse
	camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


glm::vec3 rgb(int r, int g, int b) {
	return glm::vec3(r / 255.f, g / 255.f, b / 255.f);
}

std::vector<float> generateGridVertices(int gridSize, float spacing) {
	std::vector<float> vertices;

	for (int i = -gridSize; i <= gridSize; i++) {
		float a = i * spacing;

		// Lines parallel to the x-axis
		vertices.push_back((float)-gridSize); // x1
		vertices.push_back(0.0f);     // y1
		vertices.push_back(a);    // z1

		vertices.push_back((float)gridSize); // x2
		vertices.push_back(0.0f);     // y2
		vertices.push_back(a);    // z2

		// Lines parallel to the z-axis
		vertices.push_back(a);    // x1
		vertices.push_back(0.0f);     // y1
		vertices.push_back((float)(-gridSize)); // z1

		vertices.push_back(a);    // x2
		vertices.push_back(0.0f);     // y2
		vertices.push_back((float)gridSize); // z2
	}

	// Single line on y-axis
	vertices.push_back(0.0f); // x1
	vertices.push_back((float)gridSize); // y1
	vertices.push_back(0.0f); // z1
	vertices.push_back(0.0f); // x2
	vertices.push_back((float)-gridSize); // y2
	vertices.push_back(0.0f); // z2

	return vertices;
}
