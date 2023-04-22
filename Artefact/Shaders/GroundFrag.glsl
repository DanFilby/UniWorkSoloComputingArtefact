#version 330 core

in vec3 pos;

out vec4 color;

void main(){
	
	//make plane blue at centre and fade to black to edges
	vec2 UV = (pos.xz + vec2(2.5,2.5)) / 5.0f;
	vec2 offsetFromCentre = vec2(0.5,0.5) - UV;
	float colour = (0.75 - length(offsetFromCentre)) / 2;
	color = vec4( 0.1 , 0.1 , 0.1 + colour, 1);
}