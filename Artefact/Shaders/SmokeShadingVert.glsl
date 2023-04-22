#version 330 core

layout(location = 0) in vec3 vertexPosition_modelspace;

uniform mat4 MVP;
uniform mat4 WorldMatrix;

uniform int gridWidth;
uniform float smokeBoundsRadius;

smooth out vec3 worldPos;
smooth out vec3 texturePos;

void main(){

	gl_Position = MVP * vec4(vertexPosition_modelspace,1);	
	worldPos = (WorldMatrix * vec4(vertexPosition_modelspace,1)).xyz;
	texturePos = ((vertexPosition_modelspace.xyz + vec3(smokeBoundsRadius - 0.0001)) / (smokeBoundsRadius * 2.0f));
}
