#version 330 core

in vec3 ColourPos;
in float Density;

out vec4 color;

void main(){

	//change coulour based on position
	//colour = ColourPos * Density;

	//color = vec4(0.7,0.7,0.7,Density);
	color = vec4(ColourPos, 1);
}