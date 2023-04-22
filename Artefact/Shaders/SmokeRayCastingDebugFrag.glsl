#version 330 core

uniform sampler3D VolumeTexture;

uniform vec3 CamPos;
uniform float StepSize;
uniform int MaxSamples;

smooth in vec3 worldPos;
smooth in vec3 texturePos;

out vec4 color;

void main(){

	color = vec4(texture(VolumeTexture, texturePos).x, 0, 0, 1 );
	//color = vec4(texturePos,1);
}