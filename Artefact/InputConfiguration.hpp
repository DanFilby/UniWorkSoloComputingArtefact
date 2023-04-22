#pragma once
#include <iostream>
#include <string>
#include <filesystem>

enum ArtefactMode { RealTimeSim, ReadingSim, WritingSim, IntegrationTesting };
enum RenderingMethod { Voxels, RayCasting };

const std::string ReleaseSmokeFileDirectoryPath = "Saved-Smoke/";
const std::string ReleaseExperimentDataDirectoryPath = "Experiment-Data/";


/// <summary>
/// lists each saved smoke file for the user to choose
/// </summary>
std::string GetFileName();

/// <summary>
/// creates the artefact file directories if needed
/// </summary>
void SetupDirectories();

/// <summary>
/// contains all information of the artefacts configuration
/// </summary>
struct ArtefactConfiguration {
	ArtefactMode mode = ArtefactMode::ReadingSim;
	RenderingMethod renderMethod = RenderingMethod::RayCasting;
	std::string fileName = "";

	bool runningExperiment = false;
	bool denseScenery = false;

	int simSize = 0;

	int totalWriteFrames = 0;

	void SetConfiguration(ArtefactMode& _mode, RenderingMethod& _renderer, std::string& _fileName,
		bool& _runningExperiment, bool& _denseScenery, int& _simulationSize) {
		_mode = mode;
		_renderer = renderMethod;
		_fileName = fileName;
		_runningExperiment = runningExperiment;
		_denseScenery = denseScenery;
		_simulationSize = simSize;
	}
};

//user inputs experiment settings
//TODO: change to GUI
ArtefactConfiguration ConfigureArtefact() {
	SetupDirectories();

	ArtefactConfiguration config{};

	std::string input;

	std::cout << "\nRecord Experiment Results (y/n):   ";
	std::cin >> input; std::cout << "\n";

	//when running experiment, frame time results are recorded and experiment run time is limited
	if (input == "y" || input == "Y") { config.runningExperiment = true;  config.mode = ArtefactMode::ReadingSim; }
	else { config.runningExperiment = false; }

	//if running experiment check if dense scenery is needed 
	if (config.runningExperiment) {
		std::cout << "Include Dense Scenery (y/n): ";
		std::cin >> input; std::cout << "\n";
		if (input == "y" || input == "Y") { config.denseScenery = true; }
		else { config.denseScenery = false; }
	}
	else if (!config.runningExperiment) {
		std::cout << "Input Aretfact Mode (read/write/realtime): ";
		std::cin >> input; std::cout << "\n";

		//determine artefact mode
		if (input == "read" || input == "Read") { config.mode = ArtefactMode::ReadingSim; }
		if (input == "write" || input == "Write") { config.mode = ArtefactMode::WritingSim; }
		if (input == "realtime" || input == "Realtime") { config.mode = ArtefactMode::RealTimeSim; }
	}

	//need file name for everything other than real time
	if (config.mode != ArtefactMode::RealTimeSim && config.mode != ArtefactMode::WritingSim) {
		config.fileName = GetFileName();
	}
	else if (config.mode == ArtefactMode::WritingSim) {
		std::cout << "Input Simulation File Name: ";
		std::cin >> config.fileName; std::cout << "\n";
	}

	//determine rendering method
	std::cout << "Input Rendering Method (ray/voxel): ";
	std::cin >> input; std::cout << "\n";
	if (input == "voxel" || input == "Voxel") { config.renderMethod = RenderingMethod::Voxels; }
	if (input == "ray" || input == "Ray") { config.renderMethod = RenderingMethod::RayCasting; }

	//recevied all info needed to run experiment
	if (config.runningExperiment || config.mode == ArtefactMode::ReadingSim) { return config; }

	//determine simulation resolution
	int inputResolution;
	std::cout << "Simulation Resolution: ";
	std::cin >> inputResolution; std::cout << "\n";

	//set to max and check if the input matched, lower powers of 2
	config.simSize = 256;
	//ensure resolution is a power of 2 
	for (size_t i = 2; i < 8; i++)
	{
		//calculate current power of 2
		int current = 1;
		for (size_t j = 0; j < i; j++) { current *= 2; }

		//if input is between current and the next power of two, rounding down
		if (inputResolution >= current && inputResolution < current * 2) {
			config.simSize = current;
			break;
		}
	}

	//writing specific settings
	if (config.mode == ArtefactMode::WritingSim) {
		std::cout << "Simulation Total Frames: ";
		std::cin >> config.totalWriteFrames;
	}

	return config;
}

std::string GetFileName() {
	std::string result;
	int input;

	//in release use different directory
#ifdef NDEBUG
	std::string smokeDirectoryPath = ReleaseSmokeFileDirectoryPath;

	std::filesystem::directory_iterator smokeDirectory = std::filesystem::directory_iterator(smokeDirectoryPath);
#else
	//directory of the saved smoke files, debug build
	std::string smokeDirectoryPath = "../../Saved-Smoke/";
	std::filesystem::directory_iterator smokeDirectory = std::filesystem::directory_iterator(smokeDirectoryPath);
#endif 


	//iterate over all smoke files adding an option to choose each one
	int counter = 1;
	for (const auto& smokeFile : smokeDirectory) {
		std::string smokeFileName = smokeFile.path().string().substr(smokeDirectoryPath.size());
		std::cout << "[" << counter << "] " << smokeFileName << std::endl;
		counter++;
	}

	//get correct input from user
	std::cout << "\nFile: ";
	for (size_t i = 0; i < 100; i++)
	{
		std::cin >> input;
		if (input >= 1 && input <= counter) { break; }

		std::cout << "\nIncorrect input\n";
	}

	smokeDirectory = std::filesystem::directory_iterator(smokeDirectoryPath);

	//get file name from directory
	counter = 1;
	for (const auto& smokeFile : std::filesystem::directory_iterator(smokeDirectoryPath)) {
		std::string smokeFileName = smokeFile.path().string();
		if (counter == input) {
			return smokeFileName;
		}
		counter++;
	}

	return "";
}

void SetupDirectories() {

#ifdef NDEBUG
	//create the smoke file direcory if needed
	if (!std::filesystem::is_directory(ReleaseSmokeFileDirectoryPath) || !std::filesystem::exists(ReleaseSmokeFileDirectoryPath)) {
		std::filesystem::create_directory(ReleaseSmokeFileDirectoryPath);
	}

	//create the experiment data path file direcory if needed
	if (!std::filesystem::is_directory(ReleaseExperimentDataDirectoryPath) || !std::filesystem::exists(ReleaseExperimentDataDirectoryPath)) {
		std::filesystem::create_directory(ReleaseExperimentDataDirectoryPath);
	}
#endif 

}