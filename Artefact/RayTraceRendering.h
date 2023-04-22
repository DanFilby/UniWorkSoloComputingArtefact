#pragma once

#include "Common.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "LoadShaders.hpp"
#include "FirstPersonController.h"


class RayTraceRendering
{
public:
	RayTraceRendering(float smokeBoundsWidth, int smokeGridWidth, class FirstPersonController* controller, glm::vec3* lightDir);

	/// <summary>
	/// Draws the given smoke density to screen using ray-casting
	/// </summary>
	void Draw(float* smokeDensity);

	/// <summary>
	/// Sets all properites for smoke shading
	/// </summary>
	/// <param name="lightDirection"> direction of the light </param>
	/// <param name="lightColour"> colour of the light </param>
	/// <param name="densityCoef"> effect of density on output </param>
	/// <param name="shadowCoef"> effect of shadows on output </param>
	void SetShadingProperties(glm::vec3 lightDirection, glm::vec3 lightColour, glm::vec3 skyColour,  float densityCoef, float shadowCoef, float ambientCoef);

private:

	/// <summary>
	/// generates a bounding cube for the smoke to rendered in
	/// </summary>
	void GenerateSmokeBoundingBox();
	/// <summary>
	/// creates the gl texture object, to be updated each frame with the current density
	/// </summary>
	/// <param name="densityGrid"></param>
	void GenerateTexture(float* densityGrid);
	/// <summary>
	/// updates the texture object with the current density
	/// </summary>
	/// <param name="densityGrid"></param>
	void UpdateTexture(float* densityGrid);

	//using the fps controller to get cam position and mvp
	class FirstPersonController* mFPSController;
	glm::vec3 mWorldPositon;

	//shader buffer ids
	GLuint mShaderProgramId;	
	GLuint mVertexArray;
	GLuint mElementBuffer;
	GLuint mVertexBuffer;
	GLuint mTextureId;

	//buffer for the texture data
	GLubyte* mTextureData;

	//smoke settings
	float mSmokeBoundsWidth;
	float mSmokeRadius;
	int mSmokeGridWidth;
	int mTotalSmokeCells;

	//shading
	glm::vec3* mLightDirection;
	glm::vec3 mLightColour;
	glm::vec3 mSkyColour;

	//coefficients of density and shadows
	float mDensityCoef;
	float mShadowCoef;
	float mAmbientCoef;

	//debugging
	bool bShading = true;
};

