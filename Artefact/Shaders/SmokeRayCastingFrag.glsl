#version 330 core

//used to determine if the current sampling position is outside the texture
const vec3 textureMin = vec3(0);
const vec3 textureMax = vec3(1);

//3D density texture and camera world position
uniform sampler3D VolumeTexture;
uniform vec3 CamPos;

//sampling variables
uniform float StepSize;
uniform int MaxSamples;

//fragment positions
smooth in vec3 worldPos;
smooth in vec3 texturePos;

//output colour
layout(location = 0) out vec4 vFragColor;

float RayTrace(vec3 smokeStart, vec3 dir);

void main(){
    //holds the current position of the sampler
    vec3 dataPos = texturePos;

    //calculate direction of the sampling ray
    vec3 lookDir = normalize(worldPos - CamPos); 

    //find the estimated smoke's density from a ray looking into this fragment
    float fragmentDensity = RayTrace(dataPos, lookDir);

    //set the frags colour as a flat grey using density to change opacity
    vFragColor = vec4(0.8, 0.8, 0.8, fragmentDensity);
}

//estimate density along the ray returning an approx colour and opacity
float RayTrace(vec3 rayPos, vec3 dir){

    //pre-calculate the ray-step vector
    vec3 dirStep = normalize(dir) * StepSize; 

    //acculmate density along the ray
    float acculmateDensity = 0;

    //step along the ray
    for (int i = 0; i < MaxSamples; i++) {

        //break loop if ray leaves smoke or has max density
        if (dot(sign(rayPos-textureMin),sign(textureMax-rayPos)) < 3.0 || acculmateDensity > 0.99){break;} 

        //sample the smoke's density at this position
        float density = texture(VolumeTexture, rayPos).r;

        //add density to running total
        acculmateDensity += density;

        //step along ray
        rayPos = rayPos + dirStep;
    }

    //returns the estimated density found along the ray
    return acculmateDensity;
}