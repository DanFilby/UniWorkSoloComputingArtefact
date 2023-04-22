#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in float instanceDensity;

uniform float time;

uniform mat4 MVP;

uniform int gridWidth;
uniform float smokeBoundsWidth;

out vec3 ColourPos;
out float Density;

vec3 CellPosition(){
	float x = gl_InstanceID / (gridWidth * gridWidth);
	float y = (gl_InstanceID / gridWidth) % gridWidth;
	float z = gl_InstanceID % gridWidth;
	return vec3(x + 5,y,z);
}

void main(){
	Density = instanceDensity;
	vec3 cellPos = CellPosition();

	//if density is below threshold, disregard vertex
	if(Density < 0.1){
		gl_Position = vec4(-1000,-1000,-1000,1);	
		return;
	}

	//offset each box into a grid
	vec3 posOffset = cellPos * smokeBoundsWidth / gridWidth;

	gl_Position = MVP * vec4(vertexPosition_modelspace + posOffset,1);	
}

