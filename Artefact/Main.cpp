#include <iostream>

#define GLEW_STATIC

#include "Common.h"
#include "LoadShaders.hpp"
#include "FirstPersonController.h"
#include "Scenery.h"
#include "VoxelRendering.h"
#include "RayTraceRendering.h"
#include "Smoke.h"
#include "IntegrationTests.h"
#include "DataCollection.h"
#include "InputConfiguration.hpp"

//Window
#include "WindowInitialiser.hpp"

//screen size
#define SQUARE
#if defined(SQUARE)
int WIDTH = 1000;
int HEIGHT = 1000;
#else 
int WIDTH = 1280;
int HEIGHT = 720;
#endif

//define mode to run the artefact
ArtefactMode MODE = ArtefactMode::ReadingSim;

//smoke file to write or read
std::string savedSmokeFile = "SmokeSimulation128-2";

//define rendering method
RenderingMethod RENDER_METHOD = RenderingMethod::RayCasting;

//Experiment Settings
bool bRunningExperiment = true;
bool bDenseScenery = false;
float ExperimentRunTime = 30.0f;

//Smoke Sim Settings
int SmokeGridSize = 32;
float SmokeWorldSize = 0.5f;

//program settings
float MouseSensitivity = 0.3f;

//limit frame rate, -1 -> unlimited
int FPSLimit = -1;
float PrevFrameTime = 0.0f;
float TargetFrameTime;

//when holding down step sim key, run at set rate
float SemiStepFrameTime = 0.02f;
float SemiStepPrevframe = 0.0f;

//setting flags
bool bPrintFPSToConsole = true;

bool bStepSimulation = false;
bool bEnableSmokeSimulation = true;

//window and camera controller
GLFWwindow* window;
FirstPersonController* controls;
Scenery* scenery;

//smoke
Smoke* smokeSim;
float* smokeDensityGrid;

//renderers
VoxelRendering* voxelRenderer;
RayTraceRendering* rayCastRenderer;

//testing 
IntegrationTests* integrationTesting;

ArtefactConfiguration artefactConfig;

void InputArtefactCongifuration() {
	//get configuration from user's input
	artefactConfig = ConfigureArtefact();
	artefactConfig.SetConfiguration(MODE, RENDER_METHOD, savedSmokeFile, bRunningExperiment, bDenseScenery, SmokeGridSize);
}

//initalises window, gl context, controls. checks for any errors
int AppStart(WindowInitialiser& programStart) {

	//Create the window
	programStart = WindowInitialiser();
	int errorCode = programStart.Init(WIDTH, HEIGHT);
	window = programStart.mWindow;

	//initialise controls
	FirstPersonController::window = programStart.mWindow;
	controls = new FirstPersonController(programStart.mWindow, WIDTH, HEIGHT, MouseSensitivity);

	TargetFrameTime = 1.0f / (float)FPSLimit;

	return errorCode;
}

//clears any allocated memory
void AppClose() {
	if (controls) { delete(controls); }
	if (rayCastRenderer) { delete(rayCastRenderer); }
	if (voxelRenderer) { delete(voxelRenderer); }
	if (integrationTesting) { delete(integrationTesting); }
}

//generates new smoke simulation and saves to file
void WriteNewSimulation() {
	//simulation settings
	int size = 128;
	int totalFrames = 10;

	//when in release build use user inputted settings
#ifdef NDEBUG
	size = SmokeGridSize;
	totalFrames = artefactConfig.totalWriteFrames;
#endif

	//simulate and save to file
	Smoke smoke = Smoke(size);
	smoke.CreateAndSaveSimulation(savedSmokeFile, totalFrames);
}

//sets-up artefact in the chosen mode, initalises renderer
void ArtefactSetup() {
	//Testing Mode - initalise intgration testing 
	if (MODE == ArtefactMode::IntegrationTesting) {
		integrationTesting = new IntegrationTests(window, SmokeGridSize, 0.1f);
		smokeDensityGrid = integrationTesting->mSmokeTestGrid;
	}
	//Real time simulation - intialise smoke simulation 
	else if (MODE == ArtefactMode::RealTimeSim) {
		smokeSim = new Smoke(SmokeGridSize);
		smokeDensityGrid = smokeSim->mCurrentDensity;
		smokeSim->SetAmbientVelocity(0, 0, 0);
	}
	//Reading simulation - Initalise smoke and read in the saved sim
	else if (MODE == ArtefactMode::ReadingSim) {
		smokeSim = Smoke::OpenSavedSimulation(savedSmokeFile, SmokeGridSize);
		smokeDensityGrid = smokeSim->mCurrentDensity;
	}

	//initalise voxel renderer 
	if (RENDER_METHOD == RenderingMethod::Voxels) {
		voxelRenderer = new VoxelRendering(SmokeWorldSize, SmokeGridSize, glm::vec3(0, 0, 0));
	}
	//initalise ray caster
	else if (RENDER_METHOD == RenderingMethod::RayCasting) {
		rayCastRenderer = new RayTraceRendering(SmokeWorldSize, SmokeGridSize, controls, scenery->GetLightDirection());
	}
}

//called each frame and depending on the mode updates the smoke grid 
void UpdateSmoke() {
	if (MODE == ArtefactMode::RealTimeSim) {
		smokeSim->AddDensity(SmokeGridSize / 2, 2, SmokeGridSize / 2, 10.0f);
		smokeSim->Update(controls->deltaTime);
	}
	else if (MODE == ArtefactMode::ReadingSim) {
		smokeSim->ReadNextSimulationFrame();
		smokeDensityGrid = smokeSim->mCurrentDensity;
	}
	else if (MODE == ArtefactMode::IntegrationTesting) {
		integrationTesting->Run();
	}
}

//draws the smoke grid to screen using chosen renderer
void DrawSmoke() {
	if (RENDER_METHOD == RenderingMethod::Voxels) {
		voxelRenderer->DrawVoxelsInstanced(controls->projection, controls->viewMatrix, smokeDensityGrid);
	}
	else {
		rayCastRenderer->Draw(smokeDensityGrid);
	}
}

//
void InputCheck() {
	//0: toggle mouse lock
	if (FirstPersonController::GetKeyDown(GLFW_KEY_0)) { controls->ToggleMouseLock(); }

	//1: toggle stepping simulation -- 2: to actually step simulation 
	if (FirstPersonController::GetKeyDown(GLFW_KEY_1)) { bStepSimulation = !bStepSimulation; }

	//4: add random density to smoke
	if (FirstPersonController::GetKeyDown(GLFW_KEY_4)) { smokeSim->AddRandomDensityCloud(2, 100.0f); }

	//5: clear smoke density 
	if (FirstPersonController::GetKeyDown(GLFW_KEY_5)) { smokeSim->ClearDensity(); }

	//update the smoke simulation by stepping or just every frame if stepping disabled 
	if (bEnableSmokeSimulation && (FirstPersonController::GetKeyDown(GLFW_KEY_2) || !bStepSimulation)) {
		UpdateSmoke();
	}

	//semi step simulation by
	if (bStepSimulation && controls->GetKey(GLFW_KEY_2) && glfwGetTime() - SemiStepPrevframe > SemiStepFrameTime) {
		UpdateSmoke();
		SemiStepPrevframe = glfwGetTime();
	}

	if (FirstPersonController::GetKeyDown(GLFW_KEY_8)) { scenery->ToggleLightOrbitting(); }
}

int main()
{
#ifdef NDEBUG
	//run experiment configuration startup in release build
	InputArtefactCongifuration();
#endif

	//write new simulation and end application 
	if (MODE == ArtefactMode::WritingSim) {
		WriteNewSimulation(); 
		return 0;
	}

	//initalise window and check for errors
	WindowInitialiser programStart{};
	int error = AppStart(programStart);
	if (error == -1) { return -1; }

	//setup data collection 
	DataCollection frameDataCollection = DataCollection();

	//setup back ground scenery
	scenery = new Scenery(true, true, bDenseScenery);

	//setup artefact in appropriate mode and renderer
	ArtefactSetup();

	//variables to stop experiment when desired run time is reached
	float experimentStartTime = glfwGetTime();
	bool experimentOver = false;

	//program loop
	do {
		//stop loop until dersired frame rate achieved
		if (glfwGetTime() - PrevFrameTime < TargetFrameTime) { continue; }
		PrevFrameTime = glfwGetTime();

		//clear prev frames screen
		glClearColor(0.01, 0.01, 0.01, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//call start of frame updates
		controls->Update();

		//checks for input, 1: playing sim, 2: stepping sim, 4: adding smoke, 5: clearing smoke
		InputCheck();

		//draw scenery
		scenery->Update(controls->deltaTime);
		scenery->Draw(controls->projection, controls->viewMatrix);

		//start data collection timer
		frameDataCollection.StartFrame();

		//draws smoke to screen using current renderer
		DrawSmoke();

		//update screen with newly rendered frame 
		glfwSwapBuffers(window);

		//record frame data if simulation is running
		if (bEnableSmokeSimulation && !bStepSimulation) { frameDataCollection.EndFrame(); }

		//end of frame updates
		controls->EndFrameUpdate();

		//check for input events
		glfwPollEvents();

		//stop experiment when desired run time achieved
		if (bRunningExperiment && glfwGetTime() - experimentStartTime > ExperimentRunTime) {
			experimentOver = true;
		}

	} while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0 && !experimentOver);

	//write collected data to file
	if (bRunningExperiment) {
		//define render in data collection format
		DataCollection::RenderingMethod renderer = (RENDER_METHOD == RayCasting) ? DataCollection::RayCasting : DataCollection::Voxels;
		frameDataCollection.WriteToFile(renderer, SmokeGridSize, experimentStartTime);
	}

	AppClose();
}