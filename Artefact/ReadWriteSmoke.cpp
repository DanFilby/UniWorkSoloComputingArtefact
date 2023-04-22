#include "ReadWriteSmoke.h"

#include <stdexcept>
#include <algorithm>

void ReadWriteSmoke::WriteInit(std::string fileName, int gridWidth, float* startingSmokeDensity, int blockWidth)
{
	//calculate all needed values
	mGridWidth = gridWidth;

	//amount of values in one dimension of a 'block'
	mBlockWidth = blockWidth;
	mBlockSize = blockWidth * blockWidth * blockWidth;

	//the amount of blocks in one dimension of the resulting split grid array
	mBlockArrayWidth = {};

	//check if valid configuration, throw if not 
	if (!IsValidSplit(blockWidth, mBlockArrayWidth)) {
		throw std::invalid_argument("Given block width is not compatible with current grid width");
	}

	//calculate how many 64bit longs are needed to store a bit for each block
	mFrameHeaderSize = (mBlockArrayWidth * mBlockArrayWidth * mBlockArrayWidth) / 64;
	if ((mBlockArrayWidth * mBlockArrayWidth * mBlockArrayWidth) % 64 != 0) {
		mFrameHeaderSize++;
	}

#ifdef NDEBUG

	mWriteFileStream = std::ofstream("Saved-Smoke/" + fileName + ".dat", std::ios::out | std::ios::binary | std::ios::trunc);

#else
	//create out stream to smoke data binary file 
	mWriteFileStream = std::ofstream("../../Saved-Smoke/" + fileName + ".dat", std::ios::out | std::ios::binary | std::ios::trunc);

	if (fileName == "N/A") {
		return; //used for testing
	}
	else if (!mWriteFileStream) {
		//check from another directory deeper
		if (!(mWriteFileStream = std::ofstream("../../../Saved-Smoke/" + fileName + ".dat", std::ios::out | std::ios::binary | std::ios::trunc))) {
			std::cout << "Cannot open file!" << "\n";
			throw std::invalid_argument("Cannot open file");
		}	
	}
#endif

	mFileName = fileName;

	std::cout << "Saving Smoke Settings, grid width: " << mGridWidth << ", block array width: " << mBlockArrayWidth << ", \nblock width: " << blockWidth << ", frame header size: " << mFrameHeaderSize << ", total Frames: " << 100 <<"\n\n";

	//get the file header, as block of 4-byte-ints
	uint32_t* header = EncodeFileHeader(mGridWidth, blockWidth, mBlockArrayWidth, mFrameHeaderSize, 100);

	//write the header to the file
	mWriteFileStream.write((char*)header, 8 * sizeof(uint32_t));

	//need all values to be read at the first frame, so get all block ids to add to the first frames header
	std::vector<int> fullBlockIdList = GetFullBlockIdList();

	//encode the full block id list into a frame header
	uint64_t* firstFrameHeader = EncodeFrameHeader(fullBlockIdList);

	//write the first frame header to file
	mWriteFileStream.write((char*)firstFrameHeader, mFrameHeaderSize * sizeof(uint64_t));

	//set previous frame's smoke as the starting frame
	mPreviousFrameSmoke = SplitGrid(startingSmokeDensity);

	//write the whole grid to file
	WriteFrame(mPreviousFrameSmoke, fullBlockIdList);

	//free pointers
	delete[](header);
	delete[](firstFrameHeader);
}

void ReadWriteSmoke::ReadInit(std::string fileName)
{
	//try given filename as full file path
	mReadFileStream = std::ifstream(fileName, std::ios::out | std::ios::binary);

	//otherwise try name in other directories
	if (!mReadFileStream) {
		//read file stream 
		mReadFileStream = std::ifstream("../../Saved-Smoke/" + fileName + ".dat", std::ios::out | std::ios::binary);

		//invalid file path
		if (!mReadFileStream) {
			//in unit tests go back three directories to find the smoke
			if (!(mReadFileStream = std::ifstream("../../../Saved-Smoke/" + fileName + ".dat", std::ios::out | std::ios::binary))) {
				std::cout << "Cannot open file!" << "\n";
				throw std::invalid_argument("Cannot open file");
				return;
			}
		}

	}
	
	//read file header to get info about the smoke 
	uint32_t* fileHeader = new uint32_t[8];
	mReadFileStream.read((char*)fileHeader, 8 * sizeof(uint32_t));

	//assign all info from the file header and calculate other values 
	DecodeFileHeader(fileHeader, mGridWidth, mBlockWidth, mBlockArrayWidth, mFrameHeaderSize, mSimulationTotalFrames);
	mBlockSize = mBlockWidth * mBlockWidth * mBlockWidth;

	//start smoke as blank grid
	mCurrentFrameSmokeGrid = (float*)calloc(mGridWidth * mGridWidth * mGridWidth, sizeof(float));

	//clear pointers
	delete[](fileHeader);

	mFrameHeaderBuffer = new uint64_t[mFrameHeaderSize];
	mReadBlockBuffer = new float[mBlockSize];

	std::cout << "Reading '" << fileName << "' - Settings: grid width: " << mGridWidth << ", block array width: " << mBlockArrayWidth << ", \nblock width: " << mBlockWidth << ", frame header size: " << mFrameHeaderSize << ", total Frames: " << mSimulationTotalFrames << "\n\n";
}

void ReadWriteSmoke::AddFrame(float* smokeDensity)
{
	//split the grid and find the differences between this and the previous' frames smoke
	std::vector<float*> currentFrameSmoke = SplitGrid(smokeDensity);
	std::vector<int> differenceIds = GetDifferenceSplitGrids(mPreviousFrameSmoke, currentFrameSmoke);

	//write the header which notes which blocks have changed and are written to the file
	WriteFrameHeader(differenceIds);

	//write the blocks of the current frame's smoke which have changed from the previous frame's smoke
	WriteFrame(currentFrameSmoke, differenceIds);

	//clear the previous frame's allocated memory
	ClearVec(mPreviousFrameSmoke);

	//update the preivous frame's smoke
	std::copy(currentFrameSmoke.begin(), currentFrameSmoke.end(), mPreviousFrameSmoke.begin());

	mFrameCounter++;
}

float* ReadWriteSmoke::ReadNextFrame()
{
	//read the frame header data
	mReadFileStream.read((char*)mFrameHeaderBuffer, mFrameHeaderSize * sizeof(uint64_t));

	//find the indexs of all the blocks that need updating 
	std::vector<int> blockIds = DecodeFrameHeader(mFrameHeaderBuffer);

	//if no changes between frames return last frame's smoke 
	if (blockIds.size() == 0) {
		return mCurrentFrameSmokeGrid;
	}

	//read blocks from file and apply changes to grid
	ApplyFrameChanges(blockIds);

	//return the grid
	return mCurrentFrameSmokeGrid;
}

void ReadWriteSmoke::ApplyFrameChanges(std::vector<int> changedBlocksIds)
{
	int x{}, y{}, z{}, blockIndex, blockCounter;

	//iterate over all the changed blocks
	for (size_t i = 0; i < changedBlocksIds.size(); i++)
	{
		//read the next block into memory
		mReadFileStream.read((char*)mReadBlockBuffer, mBlockSize * sizeof(float));

		//calculate the block's starting coords
		blockIndex = changedBlocksIds[i];
		x = mBlockWidth * (blockIndex % mBlockArrayWidth);
		y = mBlockWidth * ((blockIndex / (mBlockArrayWidth)) % mBlockArrayWidth);
		z = mBlockWidth * ((blockIndex / (mBlockArrayWidth * mBlockArrayWidth)));

		//iterate over whole block
		blockCounter = 0;

		//loop through all the block's values, updating the grid, in the blocks location, using the file data
		for (size_t gridZ = z; gridZ < z + mBlockWidth; gridZ++)
		{
			for (size_t gridY = y; gridY < y + mBlockWidth; gridY++)
			{
				for (size_t gridX = x; gridX < x + mBlockWidth; gridX++)
				{
					mCurrentFrameSmokeGrid[I3D(gridX, gridY, gridZ)] = mReadBlockBuffer[blockCounter];
					blockCounter++;
				}
			}
		}
	}

}

void ReadWriteSmoke::StopRead()
{
	//close file stream and free memory
	mReadFileStream.close();

	delete[](mReadBlockBuffer);
	delete[](mFrameHeaderBuffer);
	delete[](mCurrentFrameSmokeGrid);
}

void ReadWriteSmoke::StopWrite()
{
	//close file stream
	mWriteFileStream.close();
	std::fstream fileStream;

#ifdef NDEBUG
	fileStream = std::fstream("Saved-Smoke/" + mFileName + ".dat", std::ios_base::binary | std::ios_base::out | std::ios_base::in);

#else
	//reopen the file in overwrite mode and update the header with correct frame count
	fileStream = std::fstream("../../Saved-Smoke/" + mFileName + ".dat", std::ios_base::binary | std::ios_base::out | std::ios_base::in);

	if (!fileStream) {
		if (!(fileStream = std::fstream("../../../Saved-Smoke/" + mFileName + ".dat", std::ios_base::binary | std::ios_base::out | std::ios_base::in))) {
			std::cout << "Cannot reopen file!" << "\n";
			throw std::invalid_argument("Cannot open file");
		}
	}
#endif 

	//encode the header with the final frame count to block of 4-byte-ints
	uint32_t* header = EncodeFileHeader(mGridWidth, mBlockWidth, mBlockArrayWidth, mFrameHeaderSize, mFrameCounter);
	
	//set the file stream to the start of the file
	fileStream.seekp(0, std::ios_base::beg);

	//rewrite the header to the file
	fileStream.write((char*)header, 8 * sizeof(uint32_t));

	//close file
	fileStream.close();

	delete[](header);
}

void ReadWriteSmoke::WriteFrameHeader(std::vector<int> blockIndexs)
{
	//encode the block indexs to frame header format
	uint64_t* header = EncodeFrameHeader(blockIndexs);

	//write the frame's header to file
	mWriteFileStream.write((char*)header, mFrameHeaderSize * sizeof(uint64_t));
}

void ReadWriteSmoke::WriteFrame(const std::vector<float*>& currentDensity, std::vector<int> blockIndexs)
{
	//loop through all idexes of the blocks needing writing to file
	for (size_t i = 0; i < blockIndexs.size(); i++)
	{
		//get current block of values
		float* block = currentDensity[blockIndexs[i]];

		//write whole block to file
		mWriteFileStream.write((char*)block, mBlockSize * sizeof(float));
	}
}

uint32_t* ReadWriteSmoke::EncodeFileHeader(uint32_t gridWidth, uint32_t blockWidth, uint32_t blockGridWidth, uint32_t frameHeaderSize, uint32_t totalFrames)
{
	//allocate 256 bit header, using the first 20 bytes for info on format of the saved file
	uint32_t* header = (uint32_t*)malloc(8 * sizeof(uint32_t));

	//assign the first 5 values
	header[0] = gridWidth;
	header[1] = blockWidth;
	header[2] = blockGridWidth;
	header[3] = frameHeaderSize;
	header[4] = totalFrames;

	return header;
}

void ReadWriteSmoke::DecodeFileHeader(uint32_t* header, int& gridWidth, int& blockWidth, int& blockGridWidth, int& frameHeaderSizeint, int& totalFrames)
{
	//read the header info into the seperate vars
	gridWidth = header[0];
	blockWidth = header[1];
	blockGridWidth = header[2];
	frameHeaderSizeint = header[3];
	totalFrames = header[4];
}

uint64_t* ReadWriteSmoke::EncodeFrameHeader(std::vector<int> blockIndexs)
{
	uint64_t* frameHeader = (uint64_t*)calloc(mFrameHeaderSize, sizeof(uint64_t));

	for (size_t i = 0; i < blockIndexs.size(); i++)
	{
		//index of the long which this block is in
		int index = blockIndexs[i] / 64;

		//index of bit which flags this block 
		uint64_t bitIndex = blockIndexs[i] % 64;

		//set the bit at bit index to 1
		frameHeader[index] += 1LL << bitIndex;
	}

	return frameHeader;
}

std::vector<int> ReadWriteSmoke::DecodeFrameHeader(uint64_t* frameHeader)
{
	//holds the indexes of all flaged blocks
	std::vector<int> blockIds = std::vector<int>();

	for (size_t i = 0; i < mFrameHeaderSize; i++)
	{
		//contains info about 64 blocks, i.e: i = 0, info is about 0-64 blocks
		uint64_t blocksInfo = frameHeader[i];

		for (size_t j = 0; j < 64; j++)
		{
			//check if the current bit is flagged
			if ((blocksInfo >> j) & 1) {
				blockIds.push_back(j + 64 * i);
			}
		}
	}

	return blockIds;
}


bool ReadWriteSmoke::IsValidSplit(int blockWidth, int& blockArrayWidth)
{
	//get number of blocks in one dimension
	blockArrayWidth = mGridWidth / blockWidth;
	int excess = mGridWidth % blockWidth;

	//check if there are excess cells outside of the blocks
	if (excess != 0) {
		std::cout << "ERROR: there are excces cells: " << mGridWidth % blockWidth << " from " << mGridWidth << " blockwidth:" << blockWidth << "\n";
		return false;
	}

	return true;
}

bool ReadWriteSmoke::IsValidSplit(int blockWidth)
{
	//use the overloaded IsValidSplit but discard the exccess info
	int discardInt{};
	return IsValidSplit(blockWidth, discardInt);
}

std::vector<float*> ReadWriteSmoke::SplitGrid(float* grid)
{
	//create an array of blocks, using ptr to float arrays
	std::vector<float*> blocks = std::vector<float*>();

	//allocate and fill the blocks 
	for (size_t i = 0; i < mBlockArrayWidth * mBlockArrayWidth * mBlockArrayWidth; i++)
	{
		float* block = (float*)calloc(mBlockSize, sizeof(float));
		blocks.push_back(block);
	}

	//loop through grid
	for (size_t z = 0; z < mGridWidth; z++)
	{
		for (size_t x = 0; x < mGridWidth; x++)
		{
			for (size_t y = 0; y < mGridWidth; y++)
			{
				//find index of the block for the grid's current cell
				int blockIndex = (int)(x / mBlockWidth) + mBlockArrayWidth * (int)(y / mBlockWidth) + mBlockArrayWidth * mBlockArrayWidth * (int)(z / mBlockWidth);

				//find the cell in the block for the grid's current cell
				int cellIndex = (x % mBlockWidth) + (mBlockWidth * (y % mBlockWidth)) + (mBlockWidth * mBlockWidth * (z % mBlockWidth));

				//set the cell in the split grid
				blocks[blockIndex][cellIndex] = grid[I3D(x, y, z)];
			}
		}
	}

	return blocks;
}

float* ReadWriteSmoke::JoinGrids(std::vector<float*> grids)
{
	int totalGridCellCount = mBlockArrayWidth * mBlockArrayWidth * mBlockArrayWidth * mBlockSize;

	//create the resulting grid
	float* resultGrid = (float*)malloc(totalGridCellCount * sizeof(float));

	//pre calculate index offsets for the current z and y coord 
	float zBlockIndexOffset{}, yBlockIndexOffset{};
	float zCellIndexOffset{}, yCellIndexOffset{};

	for (size_t z = 0; z < mGridWidth; z++)
	{
		zBlockIndexOffset = mBlockArrayWidth * mBlockArrayWidth * (int)(z / mBlockWidth);
		zCellIndexOffset = (mBlockWidth * mBlockWidth * (z % mBlockWidth));

		for (size_t y = 0; y < mGridWidth; y++)
		{
			yBlockIndexOffset = mBlockArrayWidth * (int)(y / mBlockWidth);
			yCellIndexOffset = (mBlockWidth * (y % mBlockWidth));

			for (size_t x = 0; x < mGridWidth; x++)
			{
				//get the block and cell index of the current coord
				int blockIndex = (int)(x / mBlockWidth) + yBlockIndexOffset + zBlockIndexOffset;
				int cellIndex = (x % mBlockWidth) + yCellIndexOffset + zCellIndexOffset;

				//get the value from the split grid and set it into the result grid using the current coords
				resultGrid[I3D(x, y, z)] = grids[blockIndex][cellIndex];
			}
		}
	}
	return resultGrid;

}

int ReadWriteSmoke::GetSimulationGridWidth()
{
	return mGridWidth;
}

int ReadWriteSmoke::GetTotalFrameCount()
{
	return mSimulationTotalFrames;
}

inline std::vector<int> ReadWriteSmoke::GetDifferenceSplitGrids(const std::vector<float*>& grid1, const std::vector<float*>& grid2)
{
	//stores the indexs of all the blocks which are different 
	std::vector<int> differentBlocks = std::vector<int>();

	for (size_t i = 0; i < grid1.size(); i++)
	{
		//get current block of values from both grids
		float* block1 = grid1[i];
		float* block2 = grid2[i];

		//iterate over the whole block checking for a difference
		for (size_t j = 0; j < mBlockSize; j++)
		{
			//if difference between the two, add the block index to the list of differences
			if (abs(block1[j] - block2[j]) > 0.000001) {
				differentBlocks.push_back(i);
				break;
			}
		}
	}

	std::cout << "Block Differences: " << differentBlocks.size() << " / " << grid1.size() << "\n";

	return differentBlocks;
}

inline std::vector<int> ReadWriteSmoke::GetFullBlockIdList()
{
	//create list of indexs from 0-totalsize
	std::vector<int> fullIdList = std::vector<int>();

	for (size_t i = 0; i < mBlockArrayWidth * mBlockArrayWidth * mBlockArrayWidth; i++)
	{
		fullIdList.push_back(i);
	}

	return fullIdList;
}

inline void ReadWriteSmoke::ClearVec(std::vector<float*> vec)
{
	for (size_t i = 0; i < vec.size(); i++)
	{
		//delete dynamic array attached to pointer
		delete[]( vec[i] );
	}
}
