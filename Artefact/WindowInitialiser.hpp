#pragma once

#include "Common.h"
#include <iostream>

/**
*	Creates the window
*	sets-up up opengl context using opengl 3.3 core
*	sets-up window settings 
**/


class WindowInitialiser
{
public:
	WindowInitialiser() = default;
	~WindowInitialiser();

	/// <summary>
	///	Creates a window of the given size, sets-up opengl context
	/// </summary>
	int Init(int width, int height);

	GLFWwindow* mWindow;
};


inline int WindowInitialiser::Init(int width, int height)
{
	// Needed for core profile
	glewExperimental = true; 

	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		std::cout << "Failed to initialize GLFW\n";
		return -1;
	}
	else {
		std::cout << "initialized GLFW\n";
	}

	// 4x antialiasing, opengl 3.3
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	GLFWwindow* window;
	window = glfwCreateWindow(width, height, "Smoke", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		std::cout << "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n";
		glfwTerminate();
		return -1;
	}
	else {
		std::cout << "Opened GLFW window.\n";
	}

	// Initialize GLEW
	glfwMakeContextCurrent(window); 
	glewExperimental = true; 
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		std::cout << "Failed to initialize GLEW\n";
		return -1;
	}
	else {
		std::cout << "Initialized GLEW\n\n";
	}

	//window setup
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSwapInterval(0);
	glfwSetWindowPos(window, 100, 50);

	mWindow = window;

	//setup culling of backfaces, set triangle order to clockwise
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glEnable(GL_DEPTH_TEST);

	return 0;
}

inline WindowInitialiser::~WindowInitialiser()
{
	glfwDestroyWindow(mWindow);
}
