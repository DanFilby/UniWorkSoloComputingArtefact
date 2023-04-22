#pragma once
#include "Common.h"

#define I_3D(i,j,k) ((i) + (mGridSize * (j)) + (mGridSize * mGridSize * (k)))

class IntegrationTests
{
public:
	IntegrationTests(GLFWwindow* activeWindow, int gridSize, float frameDelay);
	~IntegrationTests();

	/// <summary>
	/// Runs the current test, checks if user pressed '9' to switch to next test
	/// </summary>
	void Run();

	/// <summary>
	/// loops through the grid setting one cell at a time
	/// </summary>
	void Test1_IncrementalSingle_Update();

	/// <summary>
	/// loops through the grid setting all cells up to the current point
	/// </summary>
	void Test2_IncrementalFull_Update();

	/// <summary>
	/// fills up grid
	/// </summary>
	void Test3_Full_Update();

	/// <summary>
	/// clears grid
	/// </summary>
	void Test4_Empty_Update();

	/// <summary>
	/// cycles full density grid values, from 0-1
	/// </summary>
	void Test5_CycleDensity_Update();

	float* mSmokeTestGrid;

private:
	/// <summary>
	/// checks if the duration between last frame meets the set frame delay
	/// </summary>
	/// <returns> true if should update </returns>
	bool CheckDelay();

	//function pointer to current test
	void (IntegrationTests::*mCurrentTestFunc)();

	
	GLFWwindow* mActiveWindow;

	float mDelayDuration;
	float prevFrameTime = 0;

	int mGridSize;
	int mTotalGridCells;

	int mCurrentTest = 0;

	//test varaibles
	int mTest1_X{}, mTest1_Y{}, mTest1_Z{};
	
	int mTest2_Count{};

	int mTest5_Count{};

};

