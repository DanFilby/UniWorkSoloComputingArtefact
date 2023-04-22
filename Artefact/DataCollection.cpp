#include "DataCollection.h"

#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>

void DataCollection::StartFrame()
{
	mCurrentframeStart = glfwGetTime();
}

void DataCollection::EndFrame()
{
	//calculate frame duration and add to trackers
	double frameDuration = glfwGetTime() - mCurrentframeStart;
	mTotalRunTime += frameDuration;
	mFrameCounter++;

	//add to the frame counter for this second
	mRunningFrameTimeCounter++;

	//every second display the frame time and average fps 
	if (bPrintToScreen && glfwGetTime() - mRunningSecondsCounter >= 1.0f) {
		//print the frame time and frames in this second
		printf("Global:  %f ms/frame | %i FPS\n", 1000.0f / double(mRunningFrameTimeCounter), mRunningFrameTimeCounter);

		//print the smokes rendering details
		printf("Smoke:  %f ms/frame | %i FPS\n\n", (double(mTotalRunTime) * 1000.0f) / mFrameCounter, int(1000.0f / ((double(mTotalRunTime) * 1000.0f) / mFrameCounter)));

		//set tracker variables
		mRunningFrameTimeCounter = 0;
		mRunningSecondsCounter += 1.0;
	}
}

void DataCollection::WriteToFile(RenderingMethod renderingMethod, int gridSize, float experimentStartTime)
{
	//experiment total run time
	double totalExperimentRunTime = glfwGetTime() - experimentStartTime;

	//used to get date of experiment
	time_t currentTime = time(0);
	tm* localTime = new tm();
	localtime_s(localTime, &currentTime);

	//write experiment data to this string stream
	std::stringstream dataStream = std::stringstream();

	//date
	dataStream << localTime->tm_mday << "/" << 1 + localTime->tm_mon << "/" << 1900 + localTime->tm_year << "\n\n";
	
	//list experiment information
	dataStream << "Renderer: " << ((renderingMethod == Voxels)? "Voxels" : "Ray-Casting") << "\n";
	dataStream << "Simulation Size: " << gridSize << "\n\n";
	dataStream << "Total Experiment Run Time: " << totalExperimentRunTime << "\n";
	dataStream << "Renderer Run Time: " << mTotalRunTime << "\n";
	dataStream << "Total Frames: " << mFrameCounter << "\n\n";

	//frame time and rate
	dataStream << "Avg Frame Time: " << (mTotalRunTime * 1000.0f) / (double)mFrameCounter << "\n";
	dataStream << "Avg Frame Rate: " << (double)mFrameCounter / mTotalRunTime<< "\n\n";

	//info in csv format
	dataStream << ((renderingMethod == Voxels) ? "Voxels" : "Ray-Casting")
		<< "," << gridSize
		<< "," << (mTotalRunTime * 1000.0f) / (double)mFrameCounter;


	//number will increase if the file already exsits
	int experimetNumber = 1;
	std::string filePath;

#ifdef NDEBUG
	std::string directoryPath = "Experiment-Data/";
#else
	std::string directoryPath = "../../Experiment-Data/";
#endif 

	//loop to check if the file exsits already and then incrementing the experiment number if so
	for (size_t i = 0; i < 64; i++)
	{
		//current file path	
		std::stringstream tempFilePath = std::stringstream();
		tempFilePath << directoryPath << "Experiment - " << localTime->tm_mday << "." << 1 + localTime->tm_mon << "." << 1900 + localTime->tm_year
			<< "-" << experimetNumber << ".dat";

		//check if file exsits already
		std::ifstream filePresent(tempFilePath.str());
		if (!filePresent.good()) {
			filePath = tempFilePath.str();
			break;
		}

		filePresent.close();
		++experimetNumber;
	}

	//write to file
	std::ofstream outfile;
	outfile.open(filePath, std::ios::out);
	outfile << dataStream.str();
	outfile.close();
}
