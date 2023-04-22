#include "VoxelRendering.h"
#include "LoadShaders.hpp"

VoxelRendering::VoxelRendering(float smokeBoundsWidth, int smokeGridWidth, glm::vec3 worldPosition, bool debugVoxels):
	mSmokeBoundsWidth(smokeBoundsWidth), mSmokeGridWidth(smokeGridWidth), mWorldPosition(worldPosition)
{
	//calculate voxel cell sizes
	mCellWidth = smokeBoundsWidth / (float)smokeGridWidth;
	mCellRadius = mCellWidth * 0.5f;

	//total amount of grid cells of the smoke 
	mSmokeGridTotalSize = mSmokeGridWidth * mSmokeGridWidth * mSmokeGridWidth;

	//if debugging voxels use a different shader
	if (debugVoxels) {
		mShaderProgramId = LoadShaders("Shaders/VoxelDebugVert.glsl", "Shaders/VoxelDebugFrag.glsl");
	}
	else {
		mShaderProgramId = LoadShaders("Shaders/SmokeVoxelVert.glsl", "Shaders/SmokeVoxelFrag.glsl");
	}

	//generates the vertices and creates the opengl buffers, for one voxel at origin positon
	CreateVoxel(glm::vec3(0,0,0));
}

void VoxelRendering::DrawVoxelsInstanced(const glm::mat4 projection, const glm::mat4& view, float* density)
{
	//cull back of voxel's faces, and enable blending
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	//set current shader and vertex array
	glUseProgram(mShaderProgramId);
	glBindVertexArray(mVoxelCells[0].vaoID);

	//set smoke density buffer
	glBindBuffer(GL_ARRAY_BUFFER, mVoxelCells[0].densityBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mSmokeGridTotalSize, &density[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//set mvp matrix using the world position
	glm::mat4 mvp = projection * view * glm::translate(glm::mat4(1.0f), mWorldPosition);
	glUniformMatrix4fv(glGetUniformLocation(mShaderProgramId, "MVP"), 1, GL_FALSE, &mvp[0][0]);

	//set smoke specific uniform values
	glUniform1i(glGetUniformLocation(mShaderProgramId, "gridWidth"), mSmokeGridWidth);
	glUniform1f(glGetUniformLocation(mShaderProgramId, "smokeBoundsWidth"), mSmokeBoundsWidth);

	//draw the instanced voxels
	glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0, mSmokeGridTotalSize);
}

void VoxelRendering::DrawVoxels(const glm::mat4 projection, const glm::mat4& view)
{
	//cull back of voxel's faces, and enable blending
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//set shaders
	glUseProgram(mShaderProgramId);

	//draw every voxel in list
	for (const VoxelCell& voxel : mVoxelCells)
	{
		//bind the current voxel
		glBindVertexArray(voxel.vaoID);

		//set mvp matrix
		glm::mat4 mvp = projection * view * glm::translate(glm::mat4(1.0f), voxel.position);
		glUniformMatrix4fv(glGetUniformLocation(mShaderProgramId, "MVP"), 1, GL_FALSE, &mvp[0][0]);
		
		//draw the current voxel
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);
	}
}

VoxelCell VoxelRendering::CreateVoxel(glm::vec3 worldPos)
{
	//vertices for a cube centred at origin, using the set cell radius 
	float vertices[] =
	{
		mCellRadius, mCellRadius, mCellRadius,
		-mCellRadius, mCellRadius, mCellRadius,
		mCellRadius, -mCellRadius, mCellRadius,
		-mCellRadius, -mCellRadius, mCellRadius,

		mCellRadius, mCellRadius, -mCellRadius,
		-mCellRadius, mCellRadius, -mCellRadius,
		mCellRadius, -mCellRadius, -mCellRadius,
		-mCellRadius, -mCellRadius, -mCellRadius,
	};

	//anti clockwise triangles to connect verts into cube
	GLuint triangles[] =
	{
		1,0,2, 3,1,2,	//front face
		4,5,7, 7,6,4,	//back face
		0,1,4, 1,5,4,	//top face
		6,7,3, 6,3,2,	//bot face
		0,4,6, 2,0,6,	//right face
		1,3,7, 7,5,1,	//left face
	};

	//create and bind vertex array
	GLuint vaoID{};
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	//create vertex buffer from vertices data
	GLuint vertexBuffer{};
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//create a buffer for the smoke's density
	unsigned int densityBuffer;
	glGenBuffers(1, &densityBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, densityBuffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), (void*)0);

	//only update once per-frame not per-vertex
	glVertexAttribDivisor(1, 1);

	//buffer for triangle indexs
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangles), triangles, GL_STATIC_DRAW);

	//creat the voxel and add to list
	VoxelCell voxelCell = VoxelCell(vaoID, vertexBuffer, densityBuffer, worldPos);
	mVoxelCells.push_back(voxelCell);

	return voxelCell;
}


