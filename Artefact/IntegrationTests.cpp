#include "IntegrationTests.h"
#include "FirstPersonController.h"
#include <iostream>

IntegrationTests::IntegrationTests(GLFWwindow* activeWindow, int gridSize, float frameDelay)
	:mActiveWindow(activeWindow), mGridSize(gridSize), mDelayDuration(frameDelay)
{
	mTotalGridCells = mGridSize * mGridSize * mGridSize;
	mSmokeTestGrid = (float*)calloc(mTotalGridCells, sizeof(float));

	mCurrentTestFunc = &IntegrationTests::Test1_IncrementalSingle_Update;
}

IntegrationTests::~IntegrationTests()
{
	if (mSmokeTestGrid) { free(mSmokeTestGrid); }
}

void IntegrationTests::Run()
{
	//change test on key press 9
	if (FirstPersonController::GetKeyDown(GLFW_KEY_9)) {
		mCurrentTest = (mCurrentTest + 1) % 5;

		switch (mCurrentTest)
		{
		case 0:
			mCurrentTestFunc = &IntegrationTests::Test1_IncrementalSingle_Update;
			std::cout << "\n Testing - Single Incremental - Press 9 for next test\n";
			break;

		case 1:
			mCurrentTestFunc = &IntegrationTests::Test2_IncrementalFull_Update;
			std::cout << "\n Testing - Full Incremental- Press 9 for next test\n";
			break;

		case 2:
			mCurrentTestFunc = &IntegrationTests::Test3_Full_Update;
			std::cout << "\n Testing - Full Smoke grid - Press 9 for next test\n";
			break;

		case 3:
			mCurrentTestFunc = &IntegrationTests::Test4_Empty_Update;
			std::cout << "\n Testing - Empty Smoke Grid - Press 9 for next test\n";
			break;

		case 4:
			mCurrentTestFunc = &IntegrationTests::Test5_CycleDensity_Update;
			std::cout << "\n Testing - Cycling Density - Press 9 for next test\n";
			break;
		default:
			break;
		}
	}

	//run current test
	(this->*mCurrentTestFunc)();
}

void IntegrationTests::Test1_IncrementalSingle_Update()
{
	//add delay between frames
	if (!CheckDelay()) {
		return;
	}

	//iterate through the grid 
	mTest1_X++;
	if (mTest1_X >= mGridSize) {
		mTest1_X = 0;
		mTest1_Y++;
	}
	if (mTest1_Y >= mGridSize) {
		mTest1_Y = 0;
		mTest1_Z++;
	}

	//set the cell at the current positon and clear rest of grid 
	for (size_t i = 0; i < mTotalGridCells; i++)
	{
		if (i == I_3D(mTest1_X, mTest1_Y, mTest1_Z)) {
			mSmokeTestGrid[i] = 0.7f;
		}
		else {
			mSmokeTestGrid[i] = 0;
		}
	}
	return;
}

void IntegrationTests::Test2_IncrementalFull_Update()
{
	//add delay between frames
	if (!CheckDelay()) {
		return;
	}

	mTest2_Count = (mTest2_Count + 1) % mTotalGridCells;

	//set density up until current count 
	for (size_t i = 0; i < mTotalGridCells; i++)
	{
		if (i <= mTest2_Count) {
			mSmokeTestGrid[i] = 0.7f;
		}
		else {
			mSmokeTestGrid[i] = 0;
		}
	}
	return;
}

void IntegrationTests::Test3_Full_Update()
{
	//add delay between frames
	if (!CheckDelay()) {
		return;
	}

	//set density up until current count 
	for (size_t i = 0; i < mTotalGridCells; i++)
	{		
		mSmokeTestGrid[i] = 0.7;
	}

	return;
}

void IntegrationTests::Test4_Empty_Update()
{
	//add delay between frames
	if (!CheckDelay()) {
		return;
	}

	//set density up until current count 
	for (size_t i = 0; i < mTotalGridCells; i++)
	{
		mSmokeTestGrid[i] = 0;
	}

	return;
}

void IntegrationTests::Test5_CycleDensity_Update()
{
	//add delay between frames
	if (!CheckDelay()) {
		return;
	}

	//create looping number from 0-1000
	mTest5_Count = (mTest5_Count + 1) % 100;

	float density = 0.01 * mTest5_Count;

	//set density up until current count 
	for (size_t i = 0; i < mTotalGridCells; i++)
	{
		mSmokeTestGrid[i] = density;
	}
}

bool IntegrationTests::CheckDelay()
{
	//add delay between frames
	if (glfwGetTime() - prevFrameTime > mDelayDuration) {

		prevFrameTime = glfwGetTime();

		if (glfwGetKey(mActiveWindow, GLFW_KEY_P) == GLFW_PRESS) {
			mDelayDuration += -0.001f;
		}
		if (glfwGetKey(mActiveWindow, GLFW_KEY_O) == GLFW_PRESS) {
			mDelayDuration += 0.001f;
		}

		return true;
	}
	return false;
}
