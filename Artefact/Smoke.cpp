#include "Smoke.h"
#include <iostream>
#include <chrono>
#include <ctime>

Smoke::Smoke(int resolution):
	mGridWidth(resolution - 2)
{
	SetupRandomGenerator();

	//total cells in each grid
	mTotalCellCount = (mGridWidth + 2) * (mGridWidth + 2) * (mGridWidth + 2);

	//allocate density grid pointers at 0
	mCurrentDensity = (float*)calloc(mTotalCellCount, sizeof(float));
	mPrevDensity = (float*)calloc(mTotalCellCount, sizeof(float));

	//allocate velocity grid pointers at 0
	mCurVelU = (float*)calloc(mTotalCellCount, sizeof(float));
	mCurVelV = (float*)calloc(mTotalCellCount, sizeof(float));
	mCurVelW = (float*)calloc(mTotalCellCount, sizeof(float));

	mPrevVelU = (float*)calloc(mTotalCellCount, sizeof(float));
	mPrevVelV = (float*)calloc(mTotalCellCount, sizeof(float));
	mPrevVelW = (float*)calloc(mTotalCellCount, sizeof(float));

	//allocate temporary buffers
	tempBufX = (float*)calloc(mTotalCellCount, sizeof(float));
	tempBufY = (float*)calloc(mTotalCellCount, sizeof(float));
	tempBufZ = (float*)calloc(mTotalCellCount, sizeof(float));
	tempBuf = (float*)calloc(mTotalCellCount, sizeof(float));

	//allocate ambient vel grids
	mAmbientVelU = (float*)calloc(mTotalCellCount, sizeof(float));	
	mAmbientVelV = (float*)calloc(mTotalCellCount, sizeof(float));
	mAmbientVelW = (float*)calloc(mTotalCellCount, sizeof(float));
}

Smoke::~Smoke()
{
	//clear allocated memory 
	free(mCurrentDensity); free(mPrevDensity);
	free(mCurVelU); free(mCurVelV); free(mCurVelW);
	free(mPrevVelU); free(mPrevVelV); free(mPrevVelW);
	free(mAmbientVelU);	free(mAmbientVelV); free(mAmbientVelW);
	free(tempBuf); free(tempBufX); free(tempBufY); free(tempBufZ);
}

void Smoke::CreateAndSaveSimulation(std::string fileName, int frames)
{
	//used to display total time taken
	auto startTime = std::chrono::system_clock::now();

	//saving interface and initalise writing to file
	ReadWriteSmoke smokeFileReadWrite{};
	smokeFileReadWrite.WriteInit(fileName, GetGridWidth(), mCurrentDensity, 8);

	SetAmbientVelocity(0, 0, 0);

	for (size_t i = 0; i < frames; i++)
	{
		auto frameStartTime = std::chrono::system_clock::now();

		//add density to bottom middle of simulation 
		AddDensity((mGridWidth + 2) / 2, 5, (mGridWidth + 2) / 2, 15.0f, 8);

		//step simulation 
		Update(0.1f);

		std::cout << "Smoke Grid Frame " << i << " / " << frames << "\n";

		std::cout << "Density: " << GetTotalDensity() << "\n";

		//write frame 
		smokeFileReadWrite.AddFrame(mCurrentDensity);
		
		//calculate timing information
		auto frameEndTime = std::chrono::system_clock::now();
		std::chrono::duration<double> frameElapsedTime = frameEndTime - frameStartTime;
		std::chrono::duration<double> totalElapsedTime = frameEndTime - startTime;

		//estimate the time remaining, in seconds
		int estimatedTimeLeft = (totalElapsedTime.count() / float(i + 1)) * (frames - (i + 1));

		//pring timing info
		std::cout << "Frame Elapsed Time: " << frameElapsedTime.count() << " s"
			<< " | Total Elapsed Time: " << int(totalElapsedTime.count()) / 60 << "m " << int(totalElapsedTime.count()) % 60 << "s"
			<< " | Remaining Time: " << estimatedTimeLeft / 60 << "m " << estimatedTimeLeft % 60 << "s"
			<< "\n\n";
	}

	//finish writing to file
	smokeFileReadWrite.StopWrite();

	auto endTime = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsedTime = endTime - startTime;

	//output successful sim and time taken
	std::cout << "\nSuccessfully Created and Saved new Smoke Simulation. Elapsed Time: " << int(elapsedTime.count()) / 60 << "m " << int(elapsedTime.count()) % 60 << "s";
}

Smoke* Smoke::OpenSavedSimulation(std::string fileName, int& gridSize)
{
	//create simulation file interface and initalise read mode
	ReadWriteSmoke* simulationLoader = new ReadWriteSmoke();
	simulationLoader->ReadInit(fileName);

	//create new smoke object which has the saved simulation loaded
	Smoke* smoke = new Smoke(simulationLoader->GetSimulationGridWidth());
	smoke->mCurrentReadSmoke = simulationLoader;
	smoke->mReadSimTotalFrames = simulationLoader->GetTotalFrameCount();
	smoke->mCurrentFile = fileName;

	//set out parameters
	gridSize = simulationLoader->GetSimulationGridWidth();
	int totalFrames = simulationLoader->GetTotalFrameCount();

	//return the new smoke obj with the saved simulation 
	return smoke;
}

void Smoke::ReadNextSimulationFrame()
{
	//std::cout << "Read Next Frame\n";

	//check not over frame limit
	mReadFrameCounter++;
	if (mReadFrameCounter >= mReadSimTotalFrames - 1) {

		//close the file and delete the reader
		mCurrentReadSmoke->StopRead();
		delete(mCurrentReadSmoke);

		//create new reader obj and re open the file
		mCurrentReadSmoke = new ReadWriteSmoke();
		mCurrentReadSmoke->ReadInit(mCurrentFile);
		mCurrentDensity = mCurrentReadSmoke->ReadNextFrame();

		mReadFrameCounter = 1;

		return;
	}

	//read next frame into density grid
	mCurrentDensity = mCurrentReadSmoke->ReadNextFrame();
}

void Smoke::Update(float deltaTime)
{
	//update velocity field 
	if (bVelocityStep) {
		VelocityStep(deltaTime);
	}

	//update smoke density
	if (bDensityStep) {
		DensityStep(deltaTime);
	}

	//std::cout << "Smoke's Total Density: " << GetTotalDensity() << "Smoke's vel: " << GetGridTotal(mCurVelV) << "\n";
}

void Smoke::DensityStep(float deltaTime)
{
	//diffusion: spread density to surrounding cells
	if (bDiffuse) {
		SWAPPOINTER(mCurrentDensity, mPrevDensity);
		Diffuse(0, mCurrentDensity, mPrevDensity, mDiffuseRate, deltaTime);
	}

	//advection: move density along velocity field 
	if (bAdvect) {
		SWAPPOINTER(mCurrentDensity, mPrevDensity);
		Advect(0, mCurrentDensity, mPrevDensity, mCurVelU, mCurVelV, mCurVelW, deltaTime);
	}	
}

void Smoke::VelocityStep(float deltaTime)
{
	//add ambient velocity
	AddSource(mCurVelU, mAmbientVelU, deltaTime);
	AddSource(mCurVelV, mAmbientVelV, deltaTime);
	AddSource(mCurVelW, mAmbientVelW, deltaTime);

	//add upward force to areas of high density
	AddBuoyancy(deltaTime);

	//add natural curls to the velocity field
	VorticityConfinement(deltaTime);

	//diffusion: spread out the velocity throughout the grid
	if (bDiffuse) {
		SwapVelPointers();

		Diffuse(1, mCurVelU, mPrevVelU, mViscosity, deltaTime);
		Diffuse(2, mCurVelV, mPrevVelV, mViscosity, deltaTime);
		Diffuse(3, mCurVelW, mPrevVelW, mViscosity, deltaTime);

		Project();
	}

	//advection: move the velocity grids along itself 
	if (bAdvect) {
		SwapVelPointers();

		Advect(1, mCurVelU, mPrevVelU, mPrevVelU, mPrevVelV, mPrevVelW, deltaTime);
		Advect(2, mCurVelV, mPrevVelV, mPrevVelU, mPrevVelV, mPrevVelW, deltaTime);
		Advect(3, mCurVelW, mPrevVelW, mPrevVelU, mPrevVelV, mPrevVelW, deltaTime);

		Project();
	}	
}

void Smoke::Diffuse(int boundaryCondition, float* current, float* previous, float rate, float dt)
{
	//eqaution constant to determine rate of diffusion including grid size and time passed
	float a = dt * rate * mGridWidth * mGridWidth * mGridWidth;

	//number of diffusion cylcles, the higher - the more accurate
	for (size_t k = 0; k < mLinearSolveTimes; k++)
	{
		for (size_t z = 1; z <= mGridWidth; z++) {

			for (size_t i = 1; i <= mGridWidth; i++) {

				for (size_t j = 1; j <= mGridWidth; j++) {

					//calculate density transferred into this grid cell, from surrounding cells
					current[INDEX3D(i, j, z)] = (previous[INDEX3D(i, j, z)] + a *
						(current[INDEX3D(i - 1, j, z)] + current[INDEX3D(i + 1, j, z)] +	//neighbouring x
							current[INDEX3D(i, j - 1, z)] + current[INDEX3D(i, j + 1, z)] +	//y
							current[INDEX3D(i, j, z - 1)] + current[INDEX3D(i, j, z + 1)])	//z
						) / (1 + 6 * a);
				}
			}
		}
		//manage grid boundary
		SetBoundary(boundaryCondition, current);
	}
}

void Smoke::Advect(int boundaryCondition, float* current, float* previous, float* u, float* v, float* w, float dt)
{
	//loop vairables;
	int i, j, k, i0, j0, k0, i1, j1, k1;
	float x, y, z, s0, t0, s1, t1, u1, u0, dtx, dty, dtz;

	//coeficients
	dtx = dty = dtz = dt * mGridWidth;

	//loop over all cells
	for (i = 1; i <= mGridWidth; i++) {
		for (j = 1; j <= mGridWidth; j++) {
			for (k = 1; k <= mGridWidth; k++) {

				//calculate predicted movement from backtracing velcoity
				x = i - dtx * u[INDEX3D(i, j, k)]; 
				y = j - dty * v[INDEX3D(i, j, k)]; 
				z = k - dtz * w[INDEX3D(i, j, k)];

				//make sure all coords are in bounds
				if (x < 0.5f) x = 0.5f; if (x > mGridWidth + 0.5f) x = mGridWidth + 0.5f;
				i0 = (int)x; i1 = i0 + 1;

				if (y < 0.5f) y = 0.5f; if (y > mGridWidth + 0.5f) y = mGridWidth + 0.5f;
				j0 = (int)y; j1 = j0 + 1;

				if (z < 0.5f) z = 0.5f; if (z > mGridWidth + 0.5f) z = mGridWidth + 0.5f;
				k0 = (int)z; k1 = k0 + 1;

				s1 = x - i0; s0 = 1 - s1;
				t1 = y - j0; t0 = 1 - t1;
				u1 = z - k0; u0 = 1 - u1;

				//update the grid with the movement 
				current[INDEX3D(i, j, k)] = s0 * (
					t0 * u0 * previous[INDEX3D(i0, j0, k0)] + t1 * u0 * previous[INDEX3D(i0, j1, k0)]
					+ t0 * u1 * previous[INDEX3D(i0, j0, k1)] + t1 * u1 * previous[INDEX3D(i0, j1, k1)]) +
					s1 * (t0 * u0 * previous[INDEX3D(i1, j0, k0)] + t1 * u0 * previous[INDEX3D(i1, j1, k0)] +
						t0 * u1 * previous[INDEX3D(i1, j0, k1)] + t1 * u1 * previous[INDEX3D(i1, j1, k1)]);
			}
		}
	}
	//manage grid boundary
	SetBoundary(boundaryCondition, current);
}

void Smoke::Project()
{
	//use temp buffers
	float* p = tempBuf, * div = tempBufX;
	float h = 1.0f / mGridWidth;

	//calculate values for div
	for (int k = 1; k <= mGridWidth; k++) {
		for (int j = 1; j <= mGridWidth; j++) {
			for (int i = 1; i <= mGridWidth; i++) {
				div[INDEX3D(i, j, k)] = -h * (
					mCurVelU[INDEX3D(i + 1, j, k)] - mCurVelU[INDEX3D(i - 1, j, k)] +
					mCurVelV[INDEX3D(i, j + 1, k)] - mCurVelV[INDEX3D(i, j - 1, k)] +
					mCurVelW[INDEX3D(i, j, k + 1)] - mCurVelW[INDEX3D(i, j, k - 1)]) / 3;
				p[INDEX3D(i, j, k)] = 0;
			}
		}
	}
	SetBoundary(0, div); SetBoundary(0, p);

	//linearly solve int p grid 
	for (int l = 0; l < mLinearSolveTimes; l++)
	{
		for (int k = 1; k <= mGridWidth; k++) {
			for (int j = 1; j <= mGridWidth; j++) {
				for (int i = 1; i <= mGridWidth; i++) {
					p[INDEX3D(i, j, k)] = (div[INDEX3D(i, j, k)] +
						p[INDEX3D(i - 1, j, k)] + p[INDEX3D(i + 1, j, k)] +
						p[INDEX3D(i, j - 1, k)] + p[INDEX3D(i, j + 1, k)] +
						p[INDEX3D(i, j, k - 1)] + p[INDEX3D(i, j, k + 1)]) / 6;
				}
			}
		}
		SetBoundary(0, p);
	}

	//intergrate into velocity field 
	for (int k = 1; k <= mGridWidth; k++) {
		for (int j = 1; j <= mGridWidth; j++) {
			for (int i = 1; i <= mGridWidth; i++) {
				mCurVelU[INDEX3D(i, j, k)] -= (p[INDEX3D(i + 1, j, k)] - p[INDEX3D(i - 1, j, k)]) / 3 / h;
				mCurVelV[INDEX3D(i, j, k)] -= (p[INDEX3D(i, j + 1, k)] - p[INDEX3D(i, j - 1, k)]) / 3 / h;
				mCurVelW[INDEX3D(i, j, k)] -= (p[INDEX3D(i, j, k + 1)] - p[INDEX3D(i, j, k - 1)]) / 3 / h;
			}
		}
	}

	SetBoundary(1, mCurVelU); SetBoundary(2, mCurVelV);
}

void Smoke::VorticityConfinement(float dt)
{
	//curl grids using the pre allocated temp buffers
	float* curlX = tempBufX, * curlY = tempBufZ, * curlZ = tempBufZ, * curl = tempBuf;

	//curl coeficient 
	float dt0 = dt * mVorticityCoef;
	
	//in loop variables 
	int index;
	float x, y, z;

	//calculate curl in each direction using current velocity field
	for (int k = 1; k < mGridWidth; k++) {
		for (int j = 1; j < mGridWidth; j++) {
			for (int i = 1; i < mGridWidth; i++) {
				index = INDEX3D(i, j, k);
				// curlx = dw/dy - dv/dz
				x = curlX[index] = (mCurVelW[INDEX3D(i, j + 1, k)] - mCurVelW[INDEX3D(i, j - 1, k)]) * 0.5f -
					(mCurVelV[INDEX3D(i, j, k + 1)] - mCurVelV[INDEX3D(i, j, k - 1)]) * 0.5f;

				// curly = du/dz - dw/dx
				y = curlY[index] = (mCurVelU[INDEX3D(i, j, k + 1)] - mCurVelU[INDEX3D(i, j, k - 1)]) * 0.5f -
					(mCurVelW[INDEX3D(i + 1, j, k)] - mCurVelW[INDEX3D(i - 1, j, k)]) * 0.5f;

				// curlz = dv/dx - du/dy
				z = curlZ[index] = (mCurVelV[INDEX3D(i + 1, j, k)] - mCurVelV[INDEX3D(i - 1, j, k)]) * 0.5f -
					(mCurVelU[INDEX3D(i, j + 1, k)] - mCurVelU[INDEX3D(i, j - 1, k)]) * 0.5f;

				// curl = |curl|
				curl[index] = sqrtf(x * x + y * y + z * z);
			}
		}
	}

	//add curl to current velocity field
	for (int k = 1; k < mGridWidth; k++) {
		for (int j = 1; j < mGridWidth; j++) {
			for (int i = 1; i < mGridWidth; i++) {
				index = INDEX3D(i, j, k);

				//confine magnitude of curls 
				float Nx = (curl[INDEX3D(i + 1, j, k)] - curl[INDEX3D(i - 1, j, k)]) * 0.5f;
				float Ny = (curl[INDEX3D(i, j + 1, k)] - curl[INDEX3D(i, j - 1, k)]) * 0.5f;
				float Nz = (curl[INDEX3D(i, j, k + 1)] - curl[INDEX3D(i, j, k - 1)]) * 0.5f;
				float len1 = 1.0f / (sqrtf(Nx * Nx + Ny * Ny + Nz * Nz) + 0.0000001f);
				Nx *= len1;
				Ny *= len1;
				Nz *= len1;

				//add to current velocity field 
				mCurVelU[index] += (Ny * curlZ[index] - Nz * curlY[index]) * dt0;
				mCurVelV[index] += (Nz * curlX[index] - Nx * curlZ[index]) * dt0;
				mCurVelW[index] += (Nx * curlY[index] - Ny * curlX[index]) * dt0;
			}
		}
	}
	
}

void Smoke::SetBoundary(int boundaryCondition, float* grid)
{
	//shorten var names for readability
	int N = mGridWidth, b = boundaryCondition;
	float* x = grid;

	for (int i = 1; i <= mGridWidth; i++)
	{
		for (int j = 1; j <= mGridWidth; j++) {
			//set the boundary faces
			x[INDEX3D(0, i, j)] = (b == 1) ? -x[INDEX3D(1, i, j)] : x[INDEX3D(1, i, j)];
			x[INDEX3D(N + 1, i, j)] = (b == 1) ? -x[INDEX3D(N, i, j)] : x[INDEX3D(N, i, j)];
			x[INDEX3D(i, 0, j)] = (b == 2) ? -x[INDEX3D(i, 1, j)] : x[INDEX3D(i, 1, j)];
			x[INDEX3D(i, N + 1, j)] = (b == 2) ? -x[INDEX3D(i, N, j)] : x[INDEX3D(i, N, j)];
			x[INDEX3D(i, j, 0)] = (b == 3) ? -x[INDEX3D(i, j, 1)] : x[INDEX3D(i, j, 1)];
			x[INDEX3D(i, j, N + 1)] = (b == 3) ? -x[INDEX3D(i, j, N)] : x[INDEX3D(i, j, N)];
		}
	}

	//set corners
	x[INDEX3D(0, 0, 0)] = (x[INDEX3D(1, 0, 0)] + x[INDEX3D(0, 1, 0)] + x[INDEX3D(0, 0, 1)]) / 3;
	x[INDEX3D(0, N + 1, 0)] = (x[INDEX3D(1, N + 1, 0)] + x[INDEX3D(0, N, 0)] + x[INDEX3D(0, N + 1, 1)]) / 3;
	x[INDEX3D(N + 1, 0, 0)] = (x[INDEX3D(N, 0, 0)] + x[INDEX3D(N + 1, 1, 0)] + x[INDEX3D(N + 1, 0, 1)]) / 3;
	x[INDEX3D(N + 1, N + 1, 0)] = (x[INDEX3D(N, N + 1, 0)] + x[INDEX3D(N + 1, N, 0)] + x[INDEX3D(N + 1, N + 1, 1)]) / 3;
	x[INDEX3D(0, 0, N + 1)] = (x[INDEX3D(1, 0, N + 1)] + x[INDEX3D(0, 1, N + 1)] + x[INDEX3D(0, 0, N)]) / 3;
	x[INDEX3D(0, N + 1, N + 1)] = (x[INDEX3D(1, N + 1, N + 1)] + x[INDEX3D(0, N, N + 1)] + x[INDEX3D(0, N + 1, N)]) / 3;
	x[INDEX3D(N + 1, 0, N + 1)] = (x[INDEX3D(N, 0, N + 1)] + x[INDEX3D(N + 1, 1, N + 1)] + x[INDEX3D(N + 1, 0, N)]) / 3;
	x[INDEX3D(N + 1, N + 1, N + 1)] = (x[INDEX3D(N, N + 1, N + 1)] + x[INDEX3D(N + 1, N, N + 1)] + x[INDEX3D(N + 1, N + 1, N)]) / 3;
}

inline void Smoke::SwapVelPointers()
{
	SWAPPOINTER(mPrevVelU, mCurVelU);
	SWAPPOINTER(mPrevVelV, mCurVelV);
	SWAPPOINTER(mPrevVelW, mCurVelW);
}

void Smoke::AddSource(float* targetGrid, float* sourceGrid, float dt)
{
	//add the source grid to the target grid
	for (size_t i = 0; i < mTotalCellCount; i++)
	{
		//targetGrid[i] += sourceGrid[i] * dt;
		targetGrid[i] = (targetGrid[i] + dt * sourceGrid[i]) / (1.0f + dt);
	}
}

void Smoke::AddBuoyancy(float dt)
{
	//add upward force whereever there's velocity
	for (int i = 0; i < mTotalCellCount; i++) {
		mCurVelV[i] += mCurrentDensity[i] * buoyancyCoef * dt;
	}
}


float Smoke::GetTotalDensity()
{
	return GetGridTotal(mCurrentDensity);
}

float Smoke::GetGridTotal(float* grid)
{
	//keep acculmative total
	float total = 0;

	//loop over whole grid adding to the total
	for (size_t i = 0; i < mTotalCellCount; i++) { total += grid[i]; }

	return total;
}

int Smoke::GetSavedSimTotalFrames()
{
	return mReadSimTotalFrames;
}

void Smoke::ClearDensity()
{
	//loop over all cells clearing thier denisty
	for (size_t i = 0; i < mTotalCellCount; i++)
	{
		mCurrentDensity[i] = 0.0f;
		mPrevDensity[i] = 0.0f;
	}
}

void Smoke::ClearVelocity()
{
	for (size_t i = 0; i < mTotalCellCount; i++)
	{
		//current velocities
		mCurVelU[i] = 0.0f;
		mCurVelV[i] = 0.0f;
		mCurVelW[i] = 0.0f;

		//previous velocities
		mPrevVelU[i] = 0.0f;
		mPrevVelV[i] = 0.0f;
		mPrevVelW[i] = 0.0f;

		//ambient velocities
		mAmbientVelU[i] = 0.0f;
		mAmbientVelV[i] = 0.0f;
		mAmbientVelW[i] = 0.0f;
	}
}

void Smoke::SetAmbientVelocity(float uVelSpeed, float vVelSpeed, float wVelSpeed)
{
	//set the ambient velocity grids with given values
	std::fill_n(mAmbientVelU, mTotalCellCount, uVelSpeed);
	std::fill_n(mAmbientVelV, mTotalCellCount, vVelSpeed);
	std::fill_n(mAmbientVelW, mTotalCellCount, wVelSpeed);
}

void Smoke::SetVelocity(float uVelSpeed, float vVelSpeed, float wVelSpeed)
{
	//set the all current velocity fields
	std::fill_n(mCurVelU, mTotalCellCount, uVelSpeed);
	std::fill_n(mCurVelV, mTotalCellCount, vVelSpeed);
	std::fill_n(mCurVelW, mTotalCellCount, wVelSpeed);

	std::fill_n(mPrevVelU, mTotalCellCount, uVelSpeed);
	std::fill_n(mPrevVelV, mTotalCellCount, vVelSpeed);
	std::fill_n(mPrevVelW, mTotalCellCount, wVelSpeed);
}

void Smoke::AddDensity(float* input)
{
	//loop through density grid adding input at each cell
	for (size_t i = 0; i < mTotalCellCount; i++)
	{
		mCurrentDensity[i] += input[i];
	}
}

void Smoke::AddDensity(int x, int y, int z, float density)
{
	//check valid coordinate
	if (INDEX3D(x, y, z) > mTotalCellCount) {
		return;
	}
	//set density at postion 
	mCurrentDensity[INDEX3D(x, y, z)] += density;
}

void Smoke::AddDensity(int x, int y, int z, float density, int radius)
{
	if (radius <= 1) {
		AddDensity(x, y, z, density);
		return;
	}

	int halfRad = radius / 2;

	for (size_t i = x - halfRad; i < x + halfRad; i++)
	{
		for (size_t j = y - halfRad; j < y + halfRad; j++)
		{
			for (size_t k = z - halfRad; k < z + halfRad; k++)
			{
				AddDensity(i, j, k, density);
			}
		}
	}

}

void Smoke::AddRandomDensityCloud(int radius, float density)
{
	//get random coords, slightly inside the grid
	int randX = randomGridBounds(randGenerator);
	int randY = randomGridBounds(randGenerator);
	int randZ = randomGridBounds(randGenerator);

	if (radius > 5) { radius = 5; }

	AddDensity(randX, randY, randZ, density);

	//add density to surrounding cells
	for (int i = 1; i < radius; i++)
	{
		AddDensity(randX + i, randY, randZ, density);
		AddDensity(randX - i, randY, randZ, density);
		AddDensity(randX, randY + i, randZ, density);
		AddDensity(randX, randY - i, randZ, density); 
		AddDensity(randX, randY, randZ + i, density);
		AddDensity(randX, randY, randZ - i, density);
	}

}

int Smoke::GetGridWidth()
{
	//grid size with the border
	return mGridWidth + 2;
}

float Smoke::GetDensityAtPoint(int x, int y, int z)
{
	return mCurrentDensity[INDEX3D(x,y,z)];
}

inline void Smoke::SetupRandomGenerator()
{
	//generate numbers within the grid with extra buffer
	int border = (mGridWidth / 15.0f);
	randGenerator = std::mt19937(dev());
	randomGridBounds = std::uniform_int_distribution<std::mt19937::result_type>(border, mGridWidth - border);
}

