#include "FirstPersonController.h"

//functions for handeling input
int64_t FirstPersonController::keysPressedThisFrame = 0;
inline inline int64_t KeyMask(int key);
void KeyPressCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);

FirstPersonController::FirstPersonController(GLFWwindow *_window, int screenWidth, int screenHeight, float mouseSensitivity):
	screenH(screenHeight), screenW(screenWidth), mouseSpeed(mouseSensitivity)
{
	mLockPositionFreeMouse = false;
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPos(window, screenW / 2.0, screenH / 2.0);

	//listen to each key pressed 
	glfwSetKeyCallback(window, KeyPressCallBack);
	keysPressedThisFrame = 0;

	Update();
	position = -2.0f * forward;
	forward = glm::vec3(0,0,-0.9);
}


void FirstPersonController::Update()
{
	//calculate delta time
	double currentTime = glfwGetTime();
	deltaTime = float(currentTime - prevTime);
	prevTime = currentTime;

	//check if not flagged to lock position and look rotation
	if (!mLockPositionFreeMouse) {
		//get look rotation from mouse and positon change from keyboard input 
		CalculateDirection(deltaTime);
		CheckmoveInput(deltaTime);
	}

	//calculate view matix
	viewMatrix = glm::lookAt(
		position,
		position + forward,
		up
	);
}

void FirstPersonController::Update(glm::vec3 pos, glm::vec3 offset, glm::vec3 upDir)
{
	//calculate delta time
	double currentTime = glfwGetTime();
	deltaTime = float(currentTime - prevTime);
	prevTime = currentTime;

	CalculateDirection(deltaTime);

	//look at target 
	viewMatrix = glm::lookAt(
		pos + offset,
		pos ,
		upDir
	);
}

void FirstPersonController::EndFrameUpdate()
{
	//reset keys pressed this frame
	keysPressedThisFrame = 0;
}


void FirstPersonController::CalculateDirection(float deltaTime)
{
	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	//reset mouse pos to centre
	glfwSetCursorPos(window, screenW / 2.0, screenH / 2.0);

	// Compute new orientation
	horizontalAngle += mouseSpeed * deltaTime * float(screenW / 2 - xpos);
	verticalAngle += mouseSpeed * deltaTime * float(screenH / 2 - ypos);

	// forward Direction : Spherical coordinates to Cartesian coordinates conversion
	forward = glm::vec3(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	// Right vector
	right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	// Up vector : perpendicular to both direction and right
	up = glm::cross(right, forward);

	//printf("%f,%f,%f\n", forward.x,forward.y,forward.z);
}

void FirstPersonController::CheckmoveInput(float deltaTime)
{
	// Move forward
	if (glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS) {
		position += forward * deltaTime * speed * (1.0f + 5.0f * GetKey(GLFW_KEY_LEFT_SHIFT));
	}
	// Move backward
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		position -= forward * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position -= right * deltaTime * speed;
	}
	//move straight up or down at a slightly slower speed
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		position.y += deltaTime * speed * 0.8f;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		position.y -= deltaTime * speed * 0.8f;
	}
}

int FirstPersonController::GetKey(int key)
{
	if (glfwGetKey(window, key) == GLFW_PRESS) {
		return 1;
	}
	return 0;
}

bool FirstPersonController::GetKeyDown(int key) 
{
	//encode the key to a mask
	int64_t mask = KeyMask(key);

	//comapare mask to keys pressed this frame
	if (FirstPersonController::keysPressedThisFrame & mask) {
		return true;
	}

	return false;
}

void FirstPersonController::ToggleMouseLock()
{
	mLockPositionFreeMouse = !mLockPositionFreeMouse;

	//if lock postion and free the mouse, set mouse mode to normal
	if (mLockPositionFreeMouse) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	else {
		//otherwise unlock controls and lock mouse to centre 
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

}

//gives the mask for a key given, used for keys pressed this frame
inline int64_t KeyMask(int key) {
	//0-9
	if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9) {
		key -= 47;
		int64_t mask = 1LL << key;
		return mask;
	}
	//a-z
	if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
		key -= 54;
		int64_t mask = 1LL << key;
		return mask;
	}
	return 0;
}

//called everytime a key is pressed. updates keys pressed this frame
void KeyPressCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//key is either a number or from a-z and is pressed
	if ((key >= GLFW_KEY_0 && key <= GLFW_KEY_9) || (key >= GLFW_KEY_A && key <= GLFW_KEY_Z)) {
		if (action == GLFW_PRESS) {

			//creates a mask to store the key as a single bit in the keyspressedthisframe variable 
			int64_t mask = KeyMask(key);
			FirstPersonController::keysPressedThisFrame += mask;
		}
	}

}
