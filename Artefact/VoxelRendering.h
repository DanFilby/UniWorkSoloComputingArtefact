#pragma once
#include "Common.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

/// <summary>
/// Holds all the information needed to draw a voxel to the screen
/// </summary>
struct VoxelCell {
public:
	VoxelCell(GLuint _vaoID, GLuint _vertexBuffer, GLuint _densityBuffer, glm::vec3 _pos) {
		vaoID = _vaoID; vertexBuffer = _vertexBuffer; position = _pos; densityBuffer = _densityBuffer; 
	};

	GLuint vaoID;
	GLuint vertexBuffer;
	GLuint densityBuffer;

	glm::vec3 position;
};

/// <summary>
/// Provides an iterface to draw volumetric data to the screen. Using instanced translucent voxels 
/// to represent each volumetric data point.
/// </summary>
class VoxelRendering
{
public:
	/// <summary>
	/// Volume renderer using voxels to represent each cell of data
	/// </summary>
	/// <param name="smokeBoundsWidth"> world size of smoke's bounding box </param>
	/// <param name="smokeGridWidth"> grid size of the smoke in one dimension </param>
	/// <param name="worldPosition"> world position of smoke </param>
	/// <param name="debugVoxels"> flag to draw in debug mode </param>
	VoxelRendering(float smokeBoundsWidth, int smokeGridWidth, glm::vec3 worldPosition, bool debugVoxels = false);

	/// <summary>
	/// Draw all active voxels of this object
	/// </summary>
	void DrawVoxels(const glm::mat4 projection, const glm::mat4& view);

	/// <summary>
	/// Draws the given volume data as instanced voxels
	/// </summary>
	/// <param name="projection"> camera projection matrix</param>
	/// <param name="view"> current camera view matrix </param>
	/// <param name="density"> pointer to smoke density grid </param>
	void DrawVoxelsInstanced(const glm::mat4 projection, const glm::mat4& view, float* density);

	/// <summary>
	/// Creates a voxel, ready to be drawn to screen, assigning vao and buffers to the returned voxel struct 
	/// </summary>
	/// <param name="worldPos"> world position of the voxel </param>
	/// <returns> the voxel data struct </returns>
	VoxelCell CreateVoxel(glm::vec3 worldPos);

private:
	int mShaderProgramId;

	//position of centre of voxels and active voxel list
	//note: instanced draw function will istance the first voxel in the list
	glm::vec3 mWorldPosition;
	std::vector<VoxelCell> mVoxelCells;

	//smoke's info 
	float mSmokeBoundsWidth;
	float mSmokeGridWidth;
	int mSmokeGridTotalSize;

	//voxel's info
	float mCellWidth;
	float mCellRadius;
};

