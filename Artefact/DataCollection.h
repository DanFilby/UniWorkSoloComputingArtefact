#pragma once

/**
* DATA COLLECTION:
* 
* Simple timer implementation
* - use StartFrame when rendering starts
* - use EndFrame when rendering finishes
* 
* Used to properly isolate computation times
* 
* Frame time functionality, if bPrintToScreen is flagged
*  - This class will output global frame time to console independent of frame start and stop locations
* 
* SAVING DATA TO FILE:
* 
* File Names:
* - use the date of experiment and number of experiments that day to ensure unique names
* - Example: "Experiment-[DATE]-[EXPERIMENT_NUMBER].dat"
*
* File Format:
* - saves: Date, Rendering method, Simulation size, Run-time, Total frames, Avg frame rate, Avg frame time
* - Example File:

6/3/2023

Renderer: Voxels
Simulation Size: 16
Run Time: 3.28038
Total Frames: 993

Avg Frame Time: 3.30351
Avg Frame Rate: 302.709

Voxels,16,3.30351

* 
* - First displays info in a readable fashion
* - Last line gives only the relevent information to paste into data analysis tool
* 
**/

class DataCollection
{
public:

	enum RenderingMethod{Voxels, RayCasting};

	DataCollection() = default;

	/// <summary>
	/// start the timer for this frame 
	/// </summary>
	void StartFrame();

	/// <summary>
	/// end the timer for this frame
	/// </summary>
	void EndFrame();

	/// <summary>
	/// write the frame data to file, named using current date and experiment number
	/// </summary>
	/// <param name="renderingMethod">  </param>
	/// <param name="gridSize">  </param>
	void WriteToFile(RenderingMethod renderingMethod, int gridSize, float experimentStartTime);

	//toggle printing the frame time to screen each second
	bool bPrintToScreen = true;

private:

	int mFrameCounter{0};
	double mTotalRunTime{0};

	double mCurrentframeStart{0};

	//used to find frame time each second
	double mRunningSecondsCounter{0};
	int mRunningFrameTimeCounter{0};
};

