#pragma once
#include "ReadWriteSmoke.h"
#include <random>

//macro to convert 3d coords to id array index
#define INDEX3D(i,j,k) ((i)+(mGridWidth+2)*(j) + (mGridWidth+2)*(mGridWidth+2)*(k))
//macro to swap two pointers 
#define SWAPPOINTER(p1,p2) {float * tmp=p1; p1=p2; p2=tmp;}

class Smoke
{
public:
	Smoke(int resolution);
	~Smoke();

	//---- READ / WRITE SIMULATION ----//

	/// <summary>
	/// generates a smoke simulation using the given frame count. then save to a file
	/// </summary>
	/// <param name="frames"> total frames of the simulation </param>
	void CreateAndSaveSimulation(std::string fileName, int frames);

	/// <summary>
	/// returns a new smoke obj loaded with the saved simulation  
	/// </summary>
	/// <param name="gridSize"> - out set to the size of the simulation </param>
	/// <param name="totalFrames"> - out set to the total number of frames</param>
	/// <returns> smoke obj containing saved simulation </returns>
	static Smoke* OpenSavedSimulation(std::string fileName, int& gridSize);

	/// <summary>
	/// reads the nexts frame's density of the currently opened saved simulation
	/// </summary>
	void ReadNextSimulationFrame();


	//---- SMOKE SIMULATION ----//

	/// <summary>
	/// updates the smoke simulation, with density and velocity steps 
	/// </summary>
	void Update(float deltaTime);

	/// <summary>
	/// update the density grid
	/// </summary>
	void DensityStep(float deltaTime);

	/// <summary>
	/// update the velocity field
	/// </summary>
	void VelocityStep(float deltaTime);

	/// <summary>
	/// Diffuses the given grid
	/// </summary>
	/// <param name="boundaryCondition"> - used in the set boundary function </param>
	/// <param name="current"> - current grid of values to diffuse </param>
	/// <param name="previous"> - previous frames grid of values </param>
	/// <param name="rate"> - diffusion rate </param>
	/// <param name="dt"> - delta time </param>
	void Diffuse(int boundaryCondition, float* current, float* previous, float rate, float dt);

	/// <summary>
	/// advects the values in the given grid, using the given velocity grids
	/// </summary>
	/// <param name="boundaryCondition"> - used in the set boundary function </param>
	/// <param name="current">- current grid of values to advect  </param>
	/// <param name="previous"> - previous frames grid of values </param>
	/// <param name="velU"> - x dir velocity grid </param>
	/// <param name="velV"> - y dir velocity grid </param>
	/// <param name="velW"> - z dir velocity grid </param>
	/// <param name="dt"> - delta time </param>
	void Advect(int boundaryCondition, float* current, float* previous, float* velU, float* velV, float* velW, float dt);

	/// <summary>
	/// projects the velocities, enforcing it to be incompressible fluid 
	/// </summary>
	void Project();

	/// <summary>
	/// adds natural curls to the velocity field
	/// </summary>
	/// <param name="dt"></param>
	void VorticityConfinement(float dt);

	/// <summary>
	/// swaps the current vel pointers to point at the prev vel grid
	/// </summary>
	inline void SwapVelPointers();

	/// <summary>
	/// manages the boundary of the smoke simulation
	/// </summary>
	void SetBoundary(int boundaryCondition, float* grid);

	/// <summary>
	/// add values from the source grid into the target grid
	/// </summary>
	void AddSource(float* targetGrid, float* sourceGrid, float dt);
	/// <summary>
	/// adds upwards velocity to the smoke based off the current density
	/// </summary>
	void AddBuoyancy(float dt);

	/// <summary>
	/// clears all density in simulation
	/// </summary>
	void ClearDensity();
	/// <summary>
	/// clears all velocity grids
	/// </summary>
	void ClearVelocity();
	/// <summary>
	/// set simulation velocity in each direction for the whole grid
	/// </summary>
	void SetAmbientVelocity(float uVelSpeed, float vVelSpeed, float wVelSpeed);
	/// <summary>
	/// Sets the velocity field to the given values
	/// </summary>
	void SetVelocity(float uVelSpeed, float vVelSpeed, float wVelSpeed);
	/// <summary>
	/// add density to the whole grid. 
	/// </summary>
	/// <param name="input"> density grid to be added to simulation </param>
	void AddDensity(float* input);
	/// <summary>
	/// add density to specific location in the simulation
	/// </summary>
	void AddDensity(int x, int y, int z, float density);
	/// <summary>
	/// add denisty to simulation in a radius
	/// </summary>
	void AddDensity(int x, int y, int z, float density, int radius);
	/// <summary>
	/// add density to random position in the simulation
	/// </summary>
	void AddRandomDensityCloud(int radius, float density);

	//get properties 
	int GetGridWidth();
	float GetDensityAtPoint(int x, int y, int z);
	float GetTotalDensity();
	/// <summary>
	/// returns the combined total of all values in the given grid
	/// </summary>
	float GetGridTotal(float* grid);
	int GetSavedSimTotalFrames();

	//flags to simulate density and velocity
	bool bDiffuse = true;
	bool bAdvect = true;
	bool bDensityStep = true;
	bool bVelocityStep = true;

	//smoke coeficients
	const float mDiffuseRate = 0.0000005f;
	const float mViscosity = 0.0000f;
	const float mVorticityCoef = 2.5f;
	const float buoyancyCoef = 0.5f;

	//total amount of cells in each grid
	int mTotalCellCount;

	//density grids
	float* mCurrentDensity, * mPrevDensity;

	//velocity grids
	float* mCurVelU, * mCurVelV, * mCurVelW;
	float* mPrevVelU, * mPrevVelV, * mPrevVelW;

	//ambient veloc
	float* mAmbientVelU, * mAmbientVelV, * mAmbientVelW;

	//temp buffers, allocated on startup to increase performance
	float* tempBufX, * tempBufY, * tempBufZ, * tempBuf;

private:

	//grid width without a boundary
	int mGridWidth;

	//amount of times to run the linear solver, higher amount -> higher accuracy and higher computation cost
	const int mLinearSolveTimes = 10;

	//read a simulation from file, max frames and counter
	std::string mCurrentFile{};
	int mReadSimTotalFrames = 0;
	int mReadFrameCounter = 0;

	//random number generation
	inline void SetupRandomGenerator();
	std::random_device dev{};
	std::mt19937 randGenerator;
	std::uniform_int_distribution<std::mt19937::result_type> randomGridBounds;

protected:
	//smoke simulation file interface
	ReadWriteSmoke* mCurrentReadSmoke;

};

