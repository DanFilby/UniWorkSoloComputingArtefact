#include "RayTraceRendering.h"

RayTraceRendering::RayTraceRendering(float smokeBoundsWidth, int smokeGridWidth, FirstPersonController* controller, glm::vec3* lightDir)
{
	//set reference to fps controller
	mFPSController = controller;

	//set smoke properties
	mSmokeGridWidth = smokeGridWidth;
	mTotalSmokeCells = mSmokeGridWidth * mSmokeGridWidth * mSmokeGridWidth;
	mSmokeBoundsWidth = smokeBoundsWidth;
	mSmokeRadius = smokeBoundsWidth / 2.0f;
	mWorldPositon = glm::vec3(0, mSmokeRadius, 0);

	//sets light settings and density coefs
	SetShadingProperties(glm::vec3(0.5, 0.5, 0), glm::vec3(0.8, 0.8, 0.7), glm::vec3(0.2, 0.2, 0.7), 10.0f, 50.0f, 0.5f);
	mLightDirection = lightDir;

	//disable shading for release build
#ifdef NDEBUG
	bShading = false;
#endif

	//open and link shaders
	if (bShading) {
		mShaderProgramId = LoadShaders("Shaders/SmokeShadingVert.glsl", "Shaders/SmokeShadingFrag.glsl");
	}
	else {
		mShaderProgramId = LoadShaders("Shaders/SmokeRayCastingVert.glsl", "Shaders/SmokeRayCastingFrag.glsl");
	}

	//genertate rendering pre-requisites
	GenerateSmokeBoundingBox();
	GenerateTexture(nullptr);
}

void RayTraceRendering::Draw(float* smokeDensity)
{
	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//bind shader and bounding box vertex array
	glUseProgram(mShaderProgramId);
	glBindVertexArray(mVertexArray);

	//set mvp - add rotation to world matrix if needed
	glm::mat4 worldMatrix = glm::translate(glm::mat4(1.0f), mWorldPositon);
	glm::mat4 mvp = mFPSController->projection * mFPSController->viewMatrix * worldMatrix;
	glUniformMatrix4fv(glGetUniformLocation(mShaderProgramId, "MVP"), 1, GL_FALSE, &mvp[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(mShaderProgramId, "WorldMatrix"), 1, GL_FALSE, &worldMatrix[0][0]);

	//set smoke grid variables
	glUniform1i(glGetUniformLocation(mShaderProgramId, "gridWidth"), mSmokeGridWidth);
	glUniform1f(glGetUniformLocation(mShaderProgramId, "smokeBoundsRadius"), mSmokeRadius);
	glUniform1f(glGetUniformLocation(mShaderProgramId, "StepSize"), 0.01f);

	//volume casting settings
	glUniform3fv(glGetUniformLocation(mShaderProgramId, "CamPos"), 1, &mFPSController->position[0]);
	glUniform1i(glGetUniformLocation(mShaderProgramId, "MaxSamples"), 200);

	if (bShading) {
		//shadow ray marching settings
		glUniform1f(glGetUniformLocation(mShaderProgramId, "ShadowStepSize"), 0.01f);

		//light and sky
		glUniform3fv(glGetUniformLocation(mShaderProgramId, "LightDir"), 1, &mLightDirection[0][0]);
		glUniform3fv(glGetUniformLocation(mShaderProgramId, "LightColour"), 1, &mLightColour[0]);
		glUniform3fv(glGetUniformLocation(mShaderProgramId, "SkyColour"), 1, &mSkyColour[0]);

		//density coefficients
		glUniform1f(glGetUniformLocation(mShaderProgramId, "DensityCoef"), mDensityCoef);
		glUniform1f(glGetUniformLocation(mShaderProgramId, "ShadowDensityCoef"), mShadowCoef);
		glUniform1f(glGetUniformLocation(mShaderProgramId, "AmbientDensityCoef"), mAmbientCoef);	
	}

	//update the 3D texture using the given smoke grid, set the texture in shader
	UpdateTexture(smokeDensity);
	GLint volumeLoc = glGetUniformLocation(mShaderProgramId, "VolumeTexture");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, mTextureId);
	glUniform1i(volumeLoc, 0);

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);
}


void RayTraceRendering::SetShadingProperties(glm::vec3 lightDirection, glm::vec3 lightColour, glm::vec3 skyColour, float densityCoef, float shadowCoef, float ambientCoef)
{
	mLightDirection = new glm::vec3(lightDirection);
	mLightColour = lightColour;
	mSkyColour = skyColour;

	mDensityCoef = densityCoef;
	mShadowCoef = shadowCoef;
	mAmbientCoef = ambientCoef;
}

void RayTraceRendering::GenerateSmokeBoundingBox()
{
	//cube vertices
	float vertices[] =
	{
		mSmokeRadius, mSmokeRadius, mSmokeRadius,
		-mSmokeRadius, mSmokeRadius, mSmokeRadius,
		mSmokeRadius, -mSmokeRadius, mSmokeRadius,
		-mSmokeRadius, -mSmokeRadius, mSmokeRadius,

		mSmokeRadius, mSmokeRadius, -mSmokeRadius,
		-mSmokeRadius, mSmokeRadius, -mSmokeRadius,
		mSmokeRadius, -mSmokeRadius, -mSmokeRadius,
		-mSmokeRadius, -mSmokeRadius, -mSmokeRadius,
	};
	
	//triangles to link verts
	GLuint triangles[] =
	{
		1,0,2, 3,1,2,	//front face
		4,5,7, 7,6,4,	//back face
		0,1,4, 1,5,4,	//top face
		6,7,3, 6,3,2,	//bot face
		0,4,6, 2,0,6,	//right face
		1,3,7, 7,5,1,	//left face
	};

	//vao
	glGenVertexArrays(1, &mVertexArray);
	glBindVertexArray(mVertexArray);

	//vertex buffer
	glGenBuffers(1, &mVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//triangles 
	glGenBuffers(1, &mElementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangles), triangles, GL_STATIC_DRAW);
}

void RayTraceRendering::GenerateTexture(float* densityGrid)
{
	//bind ray-casting vao
	glBindVertexArray(mVertexArray);

	//allocate the buffer holding the texture data
	mTextureData = (GLubyte*)calloc(mSmokeGridWidth * mSmokeGridWidth * mSmokeGridWidth, sizeof(GLubyte));

	//generate the gl texture object
	glGenTextures(1, &mTextureId);
	glBindTexture(GL_TEXTURE_3D, mTextureId);
	
	//set properties for 3d texture 
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	//set the mipmap levels
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 4);

	//create the 3d texture, with single channel which will represent density
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, mSmokeGridWidth, mSmokeGridWidth, mSmokeGridWidth, 0, GL_RED, GL_UNSIGNED_BYTE, mTextureData);
	glGenerateMipmap(GL_TEXTURE_3D);
}

void RayTraceRendering::UpdateTexture(float* densityGrid)
{
	//bind ray-casting vao
	glBindVertexArray(mVertexArray);

	//loop over the current density, converting density from 0-1, to 0-255 for texture
	for (size_t i = 0; i < mTotalSmokeCells; i++)
	{
		//clamp density to max of 1
		if (densityGrid[i] > 1.0f) {
			mTextureData[i] = 255;
		}
		else {
			mTextureData[i] = densityGrid[i] * 255;
		}
	}

	//re-generate the texture
	glBindTexture(GL_TEXTURE_3D, mTextureId);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, mSmokeGridWidth, mSmokeGridWidth, mSmokeGridWidth, 0, GL_RED, GL_UNSIGNED_BYTE, mTextureData);
}
