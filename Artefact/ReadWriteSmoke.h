#pragma once

#define I3D(i,j,k) ((i) + mGridWidth*(j) + mGridWidth*mGridWidth*(k))

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

/**
* SIMULATION COMPRESSION:
* 
* simulation comes in the form of a large flat 3d array of floats, representing the smokes's density
* usually the density changes in small areas of space and the rest of density stay constant between frames
* 
* the file only stores the differences between frames, but first splits the grid into smaller sections called 'blocks'
* eg. for a block width of 8: block 1. x(0,7), y(0,7), z(0,7) | block 2. x(8,15), y(0,7), z(0,7) etc.
* 
* when storing a frame, it splits the density and compares blocks from the preivous frame's density
* only needed to store blocks which have changed
* 
* to effciently save this to file, each frame has a header detailing which blocks need to be updated
* - this header is stored as an array of 64-bit-ints with each bit representing the index of a block
* - eg. for a grid split into 128 blocks, 2 64-bit-ints will be used
* - if a block needs updating it's corresponding bit is flagged
* 
* storing the data is simply writing the each flagged block's values to the file 
* 
* reading is done by first reading file header for simulation info
* then each frame:
*   1. read the frames header
*   2. determine which blocks need to be updated
*   3. read the blocks data back into the current density grid
*	4. repeat for next frame 
* 
* 
* SIMULATION FILE FORMAT:
* 
* 32 bytes - File header: contains the simulation information - smoke size, compression details, frame count
* 
* Frame header - block of 64-bit-ints, amount determined from file header 
* 
* Frame Data - blocks of simulation data, 32-bit-floats, amount of blocks and indexes determined from frame header
* 
* Frame Header 
* 
* Frame Data
* 
* etc.
* 
* no end file, using frame count to stop reading
**/


/// <summary>
/// Interface to read or write smoke simulation files.
/// </summary>
class ReadWriteSmoke
{
public:

	//default constructor and desctructor
	ReadWriteSmoke() = default;
	~ReadWriteSmoke() = default;

	//Writing
	/// <summary>
	/// Writing smoke to file initalisation, writes the file header (containg all information to read the simulation)
	/// and then writes the first frame. now ready to use AddFrame to continue writing the simulation. takes input for 
	/// size of blocks, this determines how small the simulation is spit into before writing to file
	/// </summary>
	/// <param name="gridWidth">- smoke grid width in one dimesion </param>
	/// <param name="startingSmokeDensity">- pointer to smoke's starting density  </param>
	/// <param name="blockWidth">- size of the blocks a simulation is split into   </param>
	void WriteInit(std::string fileName, int gridWidth, float * startingSmokeDensity, int blockWidth);

	/// <summary>
	/// adds the frame to the current simulation and saves to disk
	/// </summary>
	/// <param name="smokeDensity"> current frame's smoke density </param>
	void AddFrame(float* smokeDensity);

	//Reading
	/// <summary>
	/// reading smoke simulation initalisation, opens the file and decodes the header, setting all values
	/// ready for reading the simulation with ReadNextFrame
	/// </summary>
	void ReadInit(std::string fileName);

	/// <summary>
	/// read the next frame from the simulation file
	/// </summary>
	/// <returns> pointer to grid of the next frames density values </returns>
	float* ReadNextFrame();

	/// <summary>
	/// Reads each changed block from the file and directly applys to simulation
	/// </summary>
	/// <param name="changedBlocksIds"></param>
	void ApplyFrameChanges(std::vector<int> changedBlocksIds);

	//closing
	/// <summary>
	/// properly closes file and clears any dangling pointers 
	/// </summary>
	void StopRead();

	/// <summary>
	/// finshes writing to the file by rewriting the header to include the frame count
	/// </summary>
	void StopWrite();

	//File Header
	/// <summary>
	/// encode the simluation information to a block of 4-byte-ints
	/// </summary>
	/// <returns> a block of 8 4-byte-ints containing simulation info </returns>
	uint32_t* EncodeFileHeader(uint32_t gridWidth, uint32_t blockWidth, uint32_t blockGridWidth, uint32_t frameHeaderSize, uint32_t totalFrames);

	/// <summary>
	/// decode the simulation information from the file header to the output parameters 
	/// </summary>
	/// <param name="header"> file header as a block of 4-byte-ints </param>
	void DecodeFileHeader(uint32_t* header, int& gridWidth, int& blockWidth, int& blockGridWidth, int& frameHeaderSizeint, int& totalFrames);

	//Writing Frames
	/// <summary>
	/// Writes the data for this frame to file, writes every block which changed since last frame
	/// </summary>
	/// <param name="currentDensity"> this frame's smoke density grid </param>
	/// <param name="blockIndexs"> indexes of every block which needs to be written to file </param>
	void WriteFrame(const std::vector<float*>& currentDensity, std::vector<int> blockIndexs);

	/// <summary>
	/// writes the current frame's header to file, containing the indexes of all blocks which changed from last frames
	/// </summary>
	/// <param name="blockIndexs"> list indexes of all blocks which changed from last frames </param>
	void WriteFrameHeader(std::vector<int> blockIndexs);

	/// <summary>
	/// encode the indexes to a header of a block of 8-byte-ints  
	/// </summary>
	/// <param name="blockIndexs"> list of indexes of flagged blocks </param>
	/// <returns> a block of 8-byte-ints encoded with the block indexes </returns>
	uint64_t* EncodeFrameHeader(std::vector<int> blockIndexs);

	/// <summary>
	/// read the frame header from a block of 8-byte-ints to a list of indexes of each flagged block id
	/// </summary>
	/// <param name="frameHeader"> block of 8-byte-ints containing encoded index data </param>
	/// <returns> list of all flaged indexes </returns>
	std::vector<int> DecodeFrameHeader(uint64_t* frameHeader);

	//Splitting and Joining Grids
	/// <summary>
	/// splits the grid into a list of blocks, based on position of each cell 
	/// </summary>
	/// <param name="grid"> grid to split </param>
	/// <returns> returns a list of the split grids </returns>
	std::vector<float*> SplitGrid(float* grid);

	/// <summary>
	/// rejoin split grids into a singular array
	/// </summary>
	/// <param name="grids"> split grids </param>
	float* JoinGrids(std::vector<float*> grids);

	/// <summary>
	/// returns the indexs of all blocks that contains a value which differs between the two given grids
	/// </summary>
	/// <returns> list of indexes of each block that differs </returns>
	inline std::vector<int> GetDifferenceSplitGrids(const std::vector<float*>& grid1, const std::vector<float*>& grid2);

	//Checks
	/// <summary>
	/// checks if the wanted block width is valid
	/// </summary>
	/// <param name="blockWidth"> size of one dimension of a block </param>
	/// <returns> true if grid can be split into blocks of the given size </returns>
	bool IsValidSplit(int blockWidth);

	/// <summary>
	/// checks if the wanted block width is valid, sets the amount of blocks per grid, in one dimesion,
	/// through parameter
	/// </summary>
	/// <param name="blockWidth"> size of one dimension of a block </param>
	/// <param name="blockArrayWidth"> out - amount of blocks in one side of the grid </param>
	/// <returns> true if grid can be split into blocks of the given size </returns>
	bool IsValidSplit(int blockWidth, int& blockArrayWidth);

	//get properties for grid width and frame count
	int GetSimulationGridWidth();
	int GetTotalFrameCount();

	/// <summary>
	/// returns list of ids from 0 - max amount of blocks
	/// </summary>
	inline std::vector<int> GetFullBlockIdList();

	/// <summary>
	/// clears all pointers in given list
	/// </summary>
	inline void ClearVec(std::vector<float*> vec);

private:
	std::string mFileName;

	//file streams
	std::ofstream mWriteFileStream;
	std::ifstream mReadFileStream;

	//for writing
	std::vector<float*> mPreviousFrameSmoke;

	//for reading track smoke grid
	float* mCurrentFrameSmokeGrid;

	//buffers for reading smoke files
	uint64_t* mFrameHeaderBuffer;
	float* mReadBlockBuffer;

	int mFrameHeaderSize{};
	int mBlockSize{};

	int mGridWidth{};
	int mBlockArrayWidth{};
	int mBlockWidth{};

	int mFrameCounter{};
	int mSimulationTotalFrames{};
};