#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Shader.h"
#include "Model.h"
#include "stb_image.h"

const unsigned int winWidth = 1080;
const unsigned int winHeight = 720;
const char title[] = "assimpAnimationProject";

void fbSizeCallback(GLFWwindow* window, int w, int h);
void handleInput(GLFWwindow* window);

int main() {
	//initializing GLFW
	if (!glfwInit()) {
		std::cout << "Could not init GLFW." << std::endl;
		return -1;
	}
	
	GLFWwindow* window = glfwCreateWindow(winWidth, winHeight, title, NULL, NULL);
	
	if (window == NULL) {
		std::cout << "Could not create a GLFW window." << std::endl;
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Could not init GLAD." << std::endl;
		return -1;
	}

	glViewport(0, 0, winWidth, winHeight);
	glfwSetFramebufferSizeCallback(window, fbSizeCallback);

	stbi_set_flip_vertically_on_load(true);

	Shader shader("shaders/vertexShader.vs", "shaders/fragmentShader.fs");
	Model model("models/boblampclean.md5mesh");

	//setting up PVM matrices
	glm::mat4 modelM = glm::mat4(1.0f);

	glm::mat4 view = glm::mat4(1.0f);
	glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 pos = glm::vec3(0.0f, 30.0f, 100.0f);

	view = glm::lookAt(pos, pos + front, up);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)winWidth / (float)winHeight, 0.1f, 1000.0f);
	shader.use();
	shader.setMat4("model", modelM);
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);


	glfwSwapInterval(1);
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window)) {
		handleInput(window);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		model.draw(shader);
		model.playAnimation(glfwGetTime(), shader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void fbSizeCallback(GLFWwindow * window, int w, int h) {
	glViewport(0, 0, w, h);
}

void handleInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}