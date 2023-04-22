#version 330 core

in vec3 ColourPos;

out vec4 color;

void main(){
	color = vec4(ColourPos, 1);
}