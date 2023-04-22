#include "pch.h"
#include "CppUnitTest.h"
#include "../Artefact/Smoke.h"
#include "../Artefact/Smoke.cpp"
#include "../Artefact/ReadWriteSmoke.h"
#include "../Artefact/ReadWriteSmoke.cpp"

#include<algorithm>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace SmokeTesting
{
	TEST_CLASS(SmokeTesting)
	{
	public:

		//test if successfully creating smoke class, at the right grid size
		TEST_METHOD(Test0_SmokeConstruction)
		{
			int gridSize = 32;

			//create new smoke object
			Smoke* smoke = new Smoke(gridSize);

			//check total denisty is very near 0
			Assert::IsTrue(smoke->GetTotalDensity() < 0.001);

			//check grid size
			Assert::IsTrue(smoke->mTotalCellCount == gridSize * gridSize * gridSize);

			delete(smoke);
		}

		//checks 'GetTotalDensity' and 'AddDensity' Functions
		TEST_METHOD(Test1_SmokeHelperFunctionTests)
		{
			//create new smoke object
			Smoke* smoke = new Smoke(32);

			//check total denisty is very near 0
			Assert::IsTrue(smoke->GetTotalDensity() < 0.001);

			//add 25 units worth of density into the smoke
			smoke->AddDensity(1, 1, 1, 5.0f); 
			smoke->AddDensity(2, 2, 2, 5.0f); 
			smoke->AddDensity(3, 3, 3, 5.0f); 
			smoke->AddDensity(4, 4, 4, 5.0f); 
			smoke->AddDensity(5, 5, 5, 5.0f);

			//check smoke denisty equals the amount put in
			Assert::AreEqual(smoke->GetTotalDensity(), 25.0f);

			//check getting density function working properly
			Assert::IsTrue(smoke->GetDensityAtPoint(1, 1, 1) == 5.0f);

			delete(smoke);
		}

		//Checks smoke update not adding any density to the grid
		TEST_METHOD(Test2_EmptySmokeUpdate)
		{
			//create new smoke object
			Smoke* smoke = new Smoke(32);

			//check that the total denisty is 0 or just above
			Assert::IsTrue(smoke->GetTotalDensity() <= 0.001);

			//checking this function doesn't add any extra density
			smoke->Update(0.1f);

			//check diffusion hasn't added any density back into the smoke
			Assert::IsTrue(smoke->GetTotalDensity() <= 0.001);

			delete(smoke);
		}

		//check diffusion is moving denisty 
		TEST_METHOD(Test3_SmokeDiffusion)
		{
			//create new smoke object 
			Smoke* smoke = new Smoke(32);
			smoke->bAdvect = false;

			//check that the total denisty is 0 or just above
			Assert::IsTrue(smoke->GetTotalDensity() <= 0.001);

			//add density and track centre
			int densCentre = 15;
			smoke->AddDensity(densCentre, densCentre, densCentre, 10.0f);

			for (int i = 0; i < 5; i++) { smoke->DensityStep(0.1f); }

			//smoke diffused from centre
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre, densCentre) < 10.0f);

			//check smoke diffused in each direction
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre + 1, densCentre, densCentre) > 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre - 1, densCentre, densCentre) > 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre + 1, densCentre) > 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre - 1, densCentre) > 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre, densCentre + 1) > 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre, densCentre - 1) > 0.0f);

			delete(smoke);
		}

		//check smoke advection
		TEST_METHOD(Test4_SmokeAdvectionUp)
		{
			//create new smoke object 
			Smoke* smoke = new Smoke(32);
			smoke->bDiffuse = false;

			//set upwards velocity
			smoke->SetVelocity(0, 0.1f, 0);

			//add density and track centre
			int densCentre = 15;
			smoke->AddDensity(densCentre, densCentre, densCentre, 10.0f);

			//advect smoke
			smoke->DensityStep(0.1f);

			//smoke moved from centre to the cell above
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre, densCentre) < 10.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre + 1, densCentre) > 0.0f);

			//run advection again and check two cells above
			smoke->DensityStep(0.1f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre + 2, densCentre) > 0.0f);

			//check smoke didn't move down or across
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre - 1, densCentre) <= 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre + 1, densCentre, densCentre) <= 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre, densCentre + 1) <= 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre - 1, densCentre, densCentre) <= 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre, densCentre - 1) <= 0.0f);

			delete(smoke);
		}

		//check advection down
		TEST_METHOD(Test5_SmokeAdvectionDown)
		{
			//create new smoke object 
			Smoke* smoke = new Smoke(32);
			smoke->bDiffuse = false;

			//set upwards velocity
			smoke->SetVelocity(0, -0.1f, 0);

			//add density and track centre
			int densCentre = 15;
			smoke->AddDensity(densCentre, densCentre, densCentre, 10.0f);

			//advect smoke
			smoke->DensityStep(0.1f);

			//smoke moved from centre to the cell above
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre, densCentre) < 10.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre - 1, densCentre) > 0.0f);

			//run advection again and check two cells above
			smoke->DensityStep(0.1f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre - 2, densCentre) > 0.0f);

			//check smoke didn't move down or across
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre + 1, densCentre) <= 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre + 1, densCentre, densCentre) <= 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre, densCentre + 1) <= 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre - 1, densCentre, densCentre) <= 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre, densCentre - 1) <= 0.0f);

			delete(smoke);
		}

		//check advection in postive x direction
		TEST_METHOD(Test6_SmokeAdvectionRight)
		{
			//create new smoke object 
			Smoke* smoke = new Smoke(32);
			smoke->bDiffuse = false;

			//set upwards velocity
			smoke->SetVelocity(0.1f, 0, 0);

			//add density and track centre
			int densCentre = 15;
			smoke->AddDensity(densCentre, densCentre, densCentre, 10.0f);

			//advect smoke
			smoke->DensityStep(0.1f);

			//smoke moved from centre to the cell above
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre, densCentre) < 10.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre + 1, densCentre, densCentre) > 0.0f);

			//run advection again and check two cells above
			smoke->DensityStep(0.1f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre + 2, densCentre, densCentre) > 0.0f);

			//check smoke didn't move down or across
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre - 1, densCentre, densCentre) <= 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre + 1, densCentre) <= 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre, densCentre + 1) <= 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre - 1, densCentre, densCentre) <= 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre, densCentre - 1) <= 0.0f);

			delete(smoke);
		}

		//check advection in negative z direction
		TEST_METHOD(Test7_SmokeAdvectionForward)
		{
			//create new smoke object 
			Smoke* smoke = new Smoke(32);
			smoke->bDiffuse = false;

			//set upwards velocity
			smoke->SetVelocity(0, 0, -0.1f);

			//add density and track centre
			int densCentre = 15;
			smoke->AddDensity(densCentre, densCentre, densCentre, 10.0f);

			//advect smoke
			smoke->DensityStep(0.1f);

			//smoke moved from centre to the cell above
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre, densCentre) < 10.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre, densCentre - 1) > 0.0f);

			//run advection again and check two cells above
			smoke->DensityStep(0.1f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre, densCentre - 2) > 0.0f);

			//check smoke didn't move down or across
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre, densCentre + 1) <= 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre + 1, densCentre) <= 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre + 1, densCentre, densCentre) <= 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre - 1, densCentre, densCentre) <= 0.0f);
			Assert::IsTrue(smoke->GetDensityAtPoint(densCentre, densCentre - 1, densCentre) <= 0.0f);

			delete(smoke);
		}


	};

	TEST_CLASS(SmokeSaving)
	{
	public:
		//checks that splitting a grid and rejoining it equals the original
		TEST_METHOD(Test1_SplittingGrid)
		{
			//create smoke and add density
			Smoke* smoke = new Smoke(32);
			smoke->AddDensity(15, 15, 15, 100.0f);

			//diffuse density to populate grid
			for (size_t i = 0; i < 5; i++) { smoke->Update(0.10f); }

			//get current density grid
			float* densityGrid = smoke->mCurrentDensity;

			//interface for saving smoke to disk
			ReadWriteSmoke smokeSaving{};
			smokeSaving.WriteInit("N/A", 32, densityGrid, 8);

			//grid splitting helper function to get the block array width
			if (smokeSaving.IsValidSplit(8)) {

				//split the grid into blocks of 8 width
				std::vector<float*> densitySplit = smokeSaving.SplitGrid(densityGrid);

				//rejoin the grid together
				float* rejoinedGrid = smokeSaving.JoinGrids(densitySplit);

				//check the 'split and rejoined' grid against the starting grid
				for (size_t i = 0; i < smoke->mTotalCellCount; i++)
				{
					Assert::AreEqual(densityGrid[i], rejoinedGrid[i]);
				}

				//clear pointers
				delete(rejoinedGrid);
				for (float* splitGrid : densitySplit) { delete[](splitGrid); }
			}
			else {
				//fail test if the grid splitting is not valid
				Assert::IsTrue(false);
			}
			delete(smoke);
		}

		//checks that comaring two split grids returns the correct ids of the different ones
		TEST_METHOD(Test2_SplitGridDifferenceTest)
		{
			//create smoke and add density
			Smoke* smoke = new Smoke(32);
			smoke->ClearDensity();

			//get current density grid
			float* densityGrid = smoke->mCurrentDensity;

			//interface for saving smoke to disk
			ReadWriteSmoke smokeSaving{};
			smokeSaving.WriteInit("N/A", 32, densityGrid, 8);

			//grid splitting helper function to get the block array width
			if (smokeSaving.IsValidSplit(8)) {

				//make two splits of the grid
				std::vector<float*> densitySplit = smokeSaving.SplitGrid(densityGrid);
				std::vector<float*> blankSplit = smokeSaving.SplitGrid(densityGrid);

				//modify values of the one of the split grids
				densitySplit[0][5] = 5.0f;	//min
				densitySplit[10][5] = 5.0f;
				densitySplit[17][5] = 5.0f;
				densitySplit[20][5] = 5.0f;
				densitySplit[31][5] = 5.0f;	//max

				//get the ids of the blocks which differ
				std::vector<int> differentBlocks = smokeSaving.GetDifferenceSplitGrids(densitySplit, blankSplit);
				std::sort(differentBlocks.begin(), differentBlocks.end());

				//check ids
				Assert::AreEqual(differentBlocks[0], 0);
				Assert::AreEqual(differentBlocks[1], 10);
				Assert::AreEqual(differentBlocks[2], 17);
				Assert::AreEqual(differentBlocks[3], 20);
				Assert::AreEqual(differentBlocks[4], 31);

				//check no other blocks are different
				Assert::IsTrue(differentBlocks.size() == 5);

				for (size_t i = 0; i < densitySplit.size(); i++)
				{
					delete[](densitySplit[i]);
					delete[](blankSplit[i]);
				}

			}
			else {
				//fail test if the grid splitting is not valid
				Assert::IsTrue(false);
			}

			delete(smoke);
		}

		//checks the loading simulation function for three different files
		TEST_METHOD(Test3_LoadingSimulation) {
			int gridSize = 0;
			Smoke* smoke;

			smoke = Smoke::OpenSavedSimulation("SmokeSimulation32-1", gridSize);
			Assert::IsTrue(smoke->GetGridWidth() == 32);
			Assert::IsTrue(smoke->GetSavedSimTotalFrames() == 600);
			delete(smoke);

			smoke = Smoke::OpenSavedSimulation("SmokeSimulation64-1", gridSize);
			Assert::IsTrue(smoke->GetGridWidth() == 64);
			delete(smoke);

			smoke = Smoke::OpenSavedSimulation("SmokeSimulation128-1", gridSize);
			Assert::IsTrue(smoke->GetGridWidth() == 128);
			delete(smoke);
		}
		
		//check both witing to and reading a file
		TEST_METHOD(Test4_ReadWrite) {
			Smoke* smoke = new Smoke(32);
			smoke->ClearDensity();
			int gridTotal = smoke->mTotalCellCount;

			//these will hold a copy of the smoke simulation from the smoke object
			float* density1 = new float[gridTotal];
			float* density2 = new float[gridTotal];
			float* density3 = new float[gridTotal];
			float* density4 = new float[gridTotal];
			float* density5 = new float[gridTotal];

			//save a copy of the starting grid
			std::copy(&smoke->mCurrentDensity[0], &smoke->mCurrentDensity[gridTotal], density1);

			//interface for saving smoke to disk
			ReadWriteSmoke smokeSaving{};
			smokeSaving.WriteInit("IntegrationTest1", smoke->GetGridWidth(), &smoke->mCurrentDensity[0], 8);

			smoke->AddDensity(15, 15, 15, 100.0f);

			//update the smoke, write to file, and save a copy for each frame
			//2nd Frame 
			smoke->Update(0.1f);
			smokeSaving.AddFrame(&smoke->mCurrentDensity[0]);
			std::copy(&smoke->mCurrentDensity[0], &smoke->mCurrentDensity[gridTotal], density2);

			//3rd Frame 
			smoke->Update(0.1f);
			smokeSaving.AddFrame(&smoke->mCurrentDensity[0]);
			std::copy(&smoke->mCurrentDensity[0], &smoke->mCurrentDensity[gridTotal], density3);

			//4th Frame 
			smoke->Update(0.1f);
			smokeSaving.AddFrame(&smoke->mCurrentDensity[0]);
			std::copy(&smoke->mCurrentDensity[0], &smoke->mCurrentDensity[gridTotal], density4);

			//5th Frame 
			smoke->Update(0.1f);
			smokeSaving.AddFrame(&smoke->mCurrentDensity[0]);
			std::copy(&smoke->mCurrentDensity[0], &smoke->mCurrentDensity[gridTotal], density5);

			//finish writing to file
			smokeSaving.StopWrite();

			//create reading interface
			ReadWriteSmoke smokeReading{};
			smokeReading.ReadInit("IntegrationTest1");

			//buffers to hold a copy of each frames density
			float* readDensity1 = new float[gridTotal];
			float* readDensity2 = new float[gridTotal];
			float* readDensity3 = new float[gridTotal];
			float* readDensity4 = new float[gridTotal];
			float* readDensity5 = new float[gridTotal];

			//copy the frame's density to buffer
			float* smokesDens = smokeReading.ReadNextFrame();
			std::copy(smokesDens, smokesDens + gridTotal, readDensity1);

			//2nd frame
			smokeReading.ReadNextFrame();
			std::copy(smokesDens, smokesDens + gridTotal, readDensity2); 
			
			//3rd frame
			smokeReading.ReadNextFrame();
			std::copy(smokesDens, smokesDens + gridTotal, readDensity3);

			//4th frame 
			smokeReading.ReadNextFrame();
			std::copy(smokesDens, smokesDens + gridTotal, readDensity4);

			//5th frame
			smokeReading.ReadNextFrame();
			std::copy(smokesDens, smokesDens + gridTotal, readDensity5);

			smokeReading.StopRead();

			bool passed1 = true, passed2 = true, passed3 = true, passed4 = true, passed5 = true;

			for (size_t i = 0; i < gridTotal; i++)
			{
				if (abs(readDensity1[i] - density1[i]) > 0.00001f) {
					passed1 = false;
				}
				if (abs(readDensity2[i] - density2[i]) > 0.00001f) {
					passed2 = false;
				}
				if (abs(readDensity3[i] - density3[i]) > 0.00001f) {
					passed3 = false;
				}
				if (abs(readDensity4[i] - density4[i]) > 0.00001f) {
					passed4 = false;
				}
				if (abs(readDensity5[i] - density5[i]) > 0.00001f) {
					passed5 = false;
				}
			}

			Assert::IsTrue(passed1);
			Assert::IsTrue(passed2);
			Assert::IsTrue(passed3);
			Assert::IsTrue(passed4);
			Assert::IsTrue(passed5);

		}

	};
}