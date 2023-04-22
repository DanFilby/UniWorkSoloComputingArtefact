#include "Scenery.h"
#include "LoadShaders.hpp"
#include <glm/gtc/matrix_transform.hpp>

const float PI = 3.14159265f;

Scenery::Scenery(bool _drawGround, bool drawLightEmitter, bool _denseScenery):
	bDrawGround{_drawGround},bDenseScenery{_denseScenery}, bDrawLight{drawLightEmitter}
{
	//load shaders
	mGroundShaderProgam = LoadShaders("Shaders/GroundVert.glsl", "Shaders/GroundFrag.glsl");
	mLightShaderProgam = LoadShaders("Shaders/LightVert.glsl", "Shaders/LightFrag.glsl");

	//get the mvp uniform location, needed for each draw call
	mGroundMVPUniLoc = glGetUniformLocation(mGroundShaderProgam, "MVP");
	mLightMVPUniLoc = glGetUniformLocation(mLightShaderProgam, "MVP");

	//generate the vertex array and buffers for the ground
	if (bDenseScenery) {
		GenerateDenseGround();
	}
	else {
		GenerateGround();
	}

	//create light gl object
	GenerateLight();

	//set light starting position
	mLightOrbitDegrees = 0.70 * PI;
	OrbitLight(0);
}

void Scenery::Update(float deltaTime)
{
	if (bOrbitLight) {
		OrbitLight(deltaTime);
	}
}

void Scenery::Draw(const glm::mat4& projection, const glm::mat4& view)
{
	if (bDrawGround) {
		glDisable(GL_CULL_FACE);

		//set the shaders, bind vertex array
		glUseProgram(mGroundShaderProgam);
		glBindVertexArray(mGroundVAO);

		//calculate mvp only using camera projection and view as the plane's position and rotation are (0,0,0)
		glm::mat4 mvp = projection * view;
		glUniformMatrix4fv(mGroundMVPUniLoc, 1, GL_FALSE, &mvp[0][0]);

		//draw the ground
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGroundTriBufID);
		glDrawElements(GL_TRIANGLES, mGroundTriCount, GL_UNSIGNED_INT, (void*)0);
	}

	if (bDrawLight) {
		glDisable(GL_CULL_FACE);

		//set the shaders, bind vertex array
		glUseProgram(mLightShaderProgam);
		glBindVertexArray(mLightVAO);

		//calculate mvp only using camera projection and view as the plane's position and rotation are (0,0,0)
		glm::mat4 mvp = projection * view * mlightModelMat;
		glUniformMatrix4fv(mLightMVPUniLoc, 1, GL_FALSE, &mvp[0][0]);

		//draw the ground
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mLightTriBufID);
		glDrawElements(GL_TRIANGLES, mLightTriCount, GL_UNSIGNED_INT, (void*)0);
	}
}

void Scenery::SetLightDirection(glm::vec3 direction)
{
	mLightDir = glm::normalize(direction);
	mlightModelMat = glm::translate(glm::mat4(1), mLightDir * mLightDistance);
}

glm::vec3* Scenery::GetLightDirection()
{
	return &mLightDir;
}

void Scenery::ToggleLightOrbitting()
{
	bOrbitLight = !bOrbitLight;
}

void Scenery::GenerateGround()
{
	//flat plane vertices, with added central vertice
	GLfloat vertices[] = {
		-mGroundWidth, mGroundHeight, -mGroundWidth,
		-mGroundWidth, mGroundHeight, mGroundWidth,
		mGroundWidth, mGroundHeight, -mGroundWidth,
		mGroundWidth, mGroundHeight, mGroundWidth,
		0.0f, mGroundHeight, 0.0f
	};

	//triangles
	GLuint tris[] = {
		4,0,1, 4,1,3, 4,3,2, 4,2,0
	};

	//colours for each vertex: grey on corners and light blue for middle vertex
	GLfloat colours[] = {
		0.15,0.15,0.15,
		0.15,0.15,0.15,
		0.15,0.15,0.15,
		0.15,0.15,0.15,
		0.3,0.4,0.6,
	};

	glGenVertexArrays(1, &mGroundVAO);
	glBindVertexArray(mGroundVAO);

	//vertices
	GLuint vertexBuffer{};
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 15, &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//tris
	glGenBuffers(1, &mGroundTriBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGroundTriBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 12, &tris[0], GL_STATIC_DRAW);
	mGroundTriCount = 12;
}

void Scenery::GenerateDenseGround()
{
	//plane settings, widt: world width, density: vertices per unit width in one direction
	float width = 5, vertDensity = 1000;

	//around 25 million vertices
	int maxVerts = 25600000;

	//lists to store planes geometry
	std::vector<glm::vec3> vertices{};
	std::vector<unsigned int> tris{};

	//populate verts and tris
	FillDenseVerts(vertices, tris, width, vertDensity, maxVerts);

	std::cout << "Ground verts: " << vertices.size() << " Ground Triangles: " << tris.size() << "\n\n";

	//standard gl interface setup
	glGenVertexArrays(1, &mGroundVAO);
	glBindVertexArray(mGroundVAO);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glGenBuffers(1, &mGroundTriBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGroundTriBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, tris.size() * sizeof(unsigned int), &tris[0], GL_STATIC_DRAW);
}


void Scenery::FillDenseVerts(std::vector<glm::vec3>& vertices, std::vector<unsigned int>& tris, float width, float vertDensity, int maxVerts)
{
	//how far apart each vertex will be
	float vertSpacing = 1.0f / vertDensity;

	//check that the total vert count is within the limit, if not calculate a new one
	if (maxVerts > 0 && ((width / vertSpacing + 1) * width / vertSpacing) > maxVerts) {
		//decrease density till vert count is low enough
		for (size_t density = vertDensity; density > 0; density--)
		{
			//calculate the vert count with the new density
			float vertCount = width * width * density * density + width * density;

			vertDensity = density;
			vertSpacing = 1.0f / vertDensity;

			if (vertCount <= maxVerts) {
				break;
			}
		}
	}

	float xPos = 0, yPos = 0;

	//counter for how many verts will be in a row
	int vertsInRow = width / vertSpacing;

	for (int x = 0; x < (width / vertSpacing) + 1; x++)
	{
		for (int y = 0; y < (width / vertSpacing); y++)
		{
			float xDist = x / (float)vertsInRow;
			float yDist = y / (float)vertsInRow;

			vertices.push_back(glm::vec3(vertSpacing * x - width / 2.0, -0.05, vertSpacing * y - width / 2.0));
		}
	}

	//create square from two tris at each vertex except last row
	for (int i = 0; i < vertices.size() - vertsInRow - 1; i++)
	{
		if (i % vertsInRow == vertsInRow - 1) {
			continue;
		}

		tris.push_back(i);
		tris.push_back(i + 1);
		tris.push_back(vertsInRow + i);

		tris.push_back(vertsInRow + i);
		tris.push_back(i + 1);
		tris.push_back(vertsInRow + i + 1);
	}

	mGroundTriCount = tris.size();
}

void Scenery::GenerateLight()
{
	//generate vertices and tris for a sphere
	std::vector<glm::vec3> vertices{};
	std::vector<unsigned int> tris{};
	SphereMesh(vertices, tris);

	//standard gl interface setup
	glGenVertexArrays(1, &mLightVAO);
	glBindVertexArray(mLightVAO);

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glGenBuffers(1, &mLightTriBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mLightTriBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, tris.size() * sizeof(unsigned int), &tris[0], GL_STATIC_DRAW);
	mLightTriCount = tris.size();
}

void Scenery::SphereMesh(std::vector<glm::vec3>& vertices, std::vector<unsigned int>& tris)
{
	vertices.clear();
	tris.clear();

	//sector and step dictate how dense vert will be
	float sectorStep = 2 * PI / mLightSectorCount;
	float stackStep = PI / mLightStackCount;

	//calculate verts
	for (int i = 0; i <= mLightStackCount; ++i)
	{
		float stackAngle = PI / 2 - i * stackStep;
		float xy = mLightRadius * cosf(stackAngle); 
		float z = mLightRadius * sinf(stackAngle);

		for (int j = 0; j <= mLightSectorCount; ++j)
		{
			float sectorAngle = j * sectorStep;

			float x = xy * cosf(sectorAngle);
			float y = xy * sinf(sectorAngle);

			vertices.push_back(glm::vec3(x, y, z));
		}
	}

	//calculate tris
	for (int i = 0; i < mLightStackCount; ++i)
	{
		int k1 = i * (mLightSectorCount + 1);
		int k2 = k1 + mLightSectorCount + 1;

		for (int j = 0; j < mLightSectorCount; ++j, ++k1, ++k2)
		{
			if (i != 0)
			{
				tris.push_back(k1);
				tris.push_back(k2);
				tris.push_back(k1 + 1);
			}

			if (i != (mLightStackCount - 1))
			{
				tris.push_back(k1 + 1);
				tris.push_back(k2);
				tris.push_back(k2 + 1);
			}
		}
	}

}

void Scenery::OrbitLight(float deltaTime)
{
	//add to degrees
	mLightOrbitDegrees += mLightOrbitSpeed * deltaTime;

	//calcualte each pos, circle around y axis, and y pos will gently move up and down
	float xPos = mLightDistance * glm::cos(mLightOrbitDegrees);
	float zPos = mLightDistance * glm::sin(mLightOrbitDegrees);
	float yPos = 0.3 + glm::sin(mLightOrbitDegrees) * 0.25;

	//update light position 
	SetLightDirection(glm::vec3(xPos, yPos, zPos));
}
