#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in float instanceDensity;

uniform float time;

uniform mat4 MVP;

uniform int gridWidth;
uniform float densities[512];

out vec3 ColourPos;

vec3 CellPosition(){
	float x = gl_InstanceID / (gridWidth * gridWidth);
	float y = (gl_InstanceID / gridWidth) % gridWidth;
	float z = gl_InstanceID % gridWidth;
	return vec3(x + 5,y,z);
}

void main(){
	vec3 cellPos = CellPosition();

	//offset each box into a grid
	vec3 posOffset = cellPos * 0.011;

	//change colour based on position
	ColourPos = vec3(cellPos.x / (gridWidth), cellPos.y / (gridWidth), cellPos.z / (gridWidth));

	gl_Position = MVP * vec4(vertexPosition_modelspace + posOffset,1);	
}

