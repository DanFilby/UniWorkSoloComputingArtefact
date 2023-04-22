#pragma once
#include "Common.h"
#include <vector>

/**
*	Scenery draws background scereny to the world
*	
*	currently implented:
*	 - flat plane slightly below origin, with slight colour gradient
**/


/// <summary>
/// Render background scenery to the world.
/// </summary>
class Scenery
{
public:

	/// <summary>
	/// interface for drawing background scenery, currently draws flat plane
	/// </summary>
	Scenery(bool _drawGround, bool drawLightEmitter , bool _denseScenery);

	/// <summary>
	/// updates the dynamic elements of scenery. currently, orbit lighting
	/// </summary>
	void Update(float deltaTime);

	/// <summary>
	/// Draws all scenery
	/// </summary>
	/// <param name="projection"> camera projection </param>
	/// <param name="view"> camera's view </param>
	void Draw(const glm::mat4& projection, const glm::mat4& view);

	/// <summary>
	/// sets the direction of light
	/// </summary>
	void SetLightDirection(glm::vec3 direction);

	/// <summary>
	/// returns pointer to the current light direction
	/// </summary>
	glm::vec3* GetLightDirection();

	/// <summary>
	/// toggles the orbitting of the light, either turns on or off, depending on current state
	/// </summary>
	void ToggleLightOrbitting();

private:

	/// <summary>
	/// Genertates a flat plane at origin
	/// </summary>
	void GenerateGround();

	/// <summary>
	/// genertates highly triangle dense ground plane
	/// </summary>
	void GenerateDenseGround();

	/// <summary>
	/// populate the dense ground, vertices and tris lists
	/// </summary>
	void FillDenseVerts(std::vector<glm::vec3>& vertices, std::vector<unsigned int>& tris, float width, float vertDensity, int maxVerts);

	/// <summary>
	/// creates gl-objects for light, vao, index buf 
	/// </summary>
	void GenerateLight();

	/// <summary>
	/// genertates vertices and tris for a sphere
	/// </summary>
	void SphereMesh(std::vector<glm::vec3>& vertices, std::vector<unsigned int>& tris);

	/// <summary>
	/// orbits the light around the origin
	/// </summary>
	/// <param name="deltaTime"></param>
	void OrbitLight(float deltaTime);

	//scenery flags
	//TODO: if more background scenery are added, encode flags into bita array
	bool bDenseScenery;
	bool bDrawGround;
	bool bDrawLight;
	bool bOrbitLight = false;

	//shader & buffer ids
	int mGroundShaderProgam{};
	unsigned int mGroundVAO{};
	unsigned int mGroundTriBufID{};
	unsigned int mGroundTriCount;

	//mvp uniform location in vert shader 
	GLint mGroundMVPUniLoc{};
	GLint mLightMVPUniLoc{};

	//light mesh gl objects
	int mLightShaderProgam{};
	unsigned int mLightVAO{};
	unsigned int mLightTriBufID{};
	unsigned int mLightTriCount;

	//ground settings
	const float mGroundWidth = 5.0f;
	const float mGroundHeight = -0.1f;

	//light mesh settings
	float mLightRadius = 0.025f;
	int mLightSectorCount = 20;
	int mLightStackCount = 20;

	//light settings	
	glm::vec3 mLightDir;
	glm::vec3 mLightColour;

	//drawing light
	glm::mat4 mlightModelMat;
	float mLightDistance = 0.5;

	//light orbit
	float mLightOrbitDegrees = 0.0f;
	const float mLightOrbitSpeed = 0.8f;
};

