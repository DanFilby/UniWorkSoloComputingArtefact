#version 330 core

in float Density;

out vec4 color;

void main(){
	color = vec4(0.7,0.7,0.7, Density / 3);
}