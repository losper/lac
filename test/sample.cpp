
/* OpenGL example code - Texture
*
* apply a texture to the fullscreen quad of "Indexed VBO"
*
* Autor: Jakob Progsch
*/

#include "glad.h"
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <vector>
#include "../src/lac.hpp"


int status = 0;
lacShape *ly ;
lacImage *lx;
void glfwGetClientRect(int& width, int& height) {
	GLFWwindow* window = glfwGetCurrentContext();
	glfwGetFramebufferSize(window, &width, &height);
}
void cursor_position_callback(GLFWwindow* window, double x, double y)
{
	if (status)
	{
		ly->size(x - (ly->location().x), y - (ly->location().y));
	}
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action) {
			status = 1;
			ly->location(x, y);
			ly->size(1, 1);
		}
		else {
			status = 0;
		}
	}
}
int main() {
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	
	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(800, 480, "LearnOpenGL", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// data for a fullscreen quad (this time with texture coords)
	

	lac::instance().init(glfwGetClientRect);
	ly = new lacShape();
	lx = new lacImage();
	lx->load("E:/wall.jpg", STBI_rgb_alpha);
	lacYuv lz;
	lz.location(0, 0);
	while (!glfwWindowShouldClose(window))
	{
		// Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();

		// Render
		// Clear the colorbuffer
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw our first triangle
		
		lx->draw(GL_TRIANGLES,6,0);
		if (status) {
			ly->draw(GL_LINE_LOOP);
		}
		// Swap the screen buffers
		glfwSwapBuffers(window);
	}
	// Proper//ly de-allocate all resources once they've outlived their purpose
	/*glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);*/
	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}
