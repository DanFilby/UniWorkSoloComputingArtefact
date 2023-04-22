#pragma once
#include "Common.h"
#include <glm/gtc/matrix_transform.hpp>

/**
* First person controller:
* 
* class uses standard fps controls to move the camera around the world
* visualised by updation the view matrix
* 
* contains implementations for key presses and active keys pressed
**/


/// <summary>
/// First person camera controller, sets view matrix each frame
/// using world postion and rotation. static functions for key presses 
/// </summary>
class FirstPersonController
{
public:

	FirstPersonController(GLFWwindow* _window, int screenWidth, int screenHeight, float mouseSensitivity);

	/// <summary>
	/// update the camera with fps controls
	/// </summary>
	void Update();

	/// <summary>
	/// Update the camera to look at the given position from an offset
	/// </summary>
	/// <param name="pos"> centre position </param>
	/// <param name="offset"> cam offset from centre position </param>
	/// <param name="upDir"> up direction of camera </param>
	void Update(glm::vec3 pos, glm::vec3 offset, glm::vec3 upDir);

	/// <summary>
	/// updates actions that happened this frame
	/// </summary>
	void EndFrameUpdate();

	/// <summary>
	/// check if given key is currently pressed
	/// </summary>
	/// <param name="key"> glfw key code </param>
	/// <returns> 1 if key is currently pressed, 0 if not </returns>
	static int GetKey(int key);

	/// <summary>
	/// checks if the given key was pressed this frame
	/// </summary>
	/// <param name="key"> glfw key code </param>
	/// <returns> true if given key was pressed this frame, false if not </returns>
	static bool GetKeyDown(int key);

	//stores the keys pressed each frame as bits
	static int64_t keysPressedThisFrame;

	/// <summary>
	/// toggles lock for fps controls and mouse. either lock the mouse and hide
	/// or unlock and free the mouse 
	/// </summary>
	void ToggleMouseLock();

	//static, so static key functions can reference
	inline static GLFWwindow* window;

	double deltaTime = 0.0;

	//position and rotaion 
	glm::vec3 position = glm::vec3(0, 0, -5);
	glm::vec3 forward{};
	glm::vec3 right{};
	glm::vec3 up{};

	//current camera view matrix
	glm::mat4 viewMatrix{};

	//projection matrix: 45° Field of View, display range : 0.1 unit <-> 100 units
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenW / (float)screenH, 0.1f, 100.0f);

private:

	/// <summary>
	/// update rotations using mouse movements
	/// </summary>
	void CalculateDirection(float deltaTime);

	/// <summary>
	/// update position using wasd, space and ctrl for up and down
	/// </summary>
	/// <param name="deltaTime"></param>
	void CheckmoveInput(float deltaTime);

	int screenW, screenH;

	double prevTime = 0.0;

	// horizontal angle : toward -Z
	float horizontalAngle = -3.14f;
	// vertical angle : 0, look at the horizon
	float verticalAngle = 0.0f;
	// Initial Field of View
	float initialFoV = 45.0f;

	float speed = 1.0f;
	float mouseSpeed = 0.05f;

	int mouseXpos = 0;
	int mouseYPos = 0;

	//locks the camera position and rotaion and frees mouse when true
	bool mLockPositionFreeMouse;
};

