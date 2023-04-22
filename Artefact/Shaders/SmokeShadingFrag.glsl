#version 330 core

//used to determine if the current sampling position is outside the texture
const vec3 textureMin = vec3(-0.001);
const vec3 textureMax = vec3(1);

uniform float smokeBoundsRadius;

//3D density texture and camera world position
uniform sampler3D VolumeTexture;
uniform vec3 CamPos;

//sampling variables
uniform float StepSize;
uniform float ShadowStepSize;
uniform int MaxSamples;

//shading settings
uniform vec3 LightDir;
uniform vec3 LightColour;
uniform vec3 SkyColour;

//shading coefficients
uniform float DensityCoef;
uniform float ShadowDensityCoef;
uniform float AmbientDensityCoef;

//fragment positions
smooth in vec3 worldPos;
smooth in vec3 texturePos;

//output colour
layout(location = 0) out vec4 vFragColor;

vec4 RayTraceShading(vec3 startPoint, vec3 rayDir );

void main(){

    //holds the current position of the sampler
    vec3 dataPos = texturePos;

    //calculate direction of the sampling ray
    vec3 lookDir = normalize(worldPos - CamPos); 

    //results from ray trace on this fragment, finds colour and opacity from sampling smoke's denisty
    vec4 rayTraceResults = RayTraceShading(dataPos,lookDir);

    //contrive colour and opacity from ray trace results
    vec3 colour = rayTraceResults.xyz * LightColour;
    float alpha = 1 - rayTraceResults.w;
    
    //set frag 
    vFragColor = vec4(colour, alpha);
}

//estimate shading and opacity by tracing a ray through the smoke's density
vec4 RayTraceShading(vec3 startPoint, vec3 rayDir ){

    //pre-calculate density and shaow density multipliers
    float densityMulti =  DensityCoef * StepSize;
    float shadowDensityMulti = ShadowDensityCoef * StepSize;

    //if a shadow reaches this threshold it can stop being calculated, as max
    float shadowThreshold = -log(0.0001) / shadowDensityMulti;

    //setup rays
    vec3 rayPoint = startPoint;
    vec3 stepRay = rayDir * StepSize;
    vec3 lightRay = normalize(LightDir) * ShadowStepSize;

    //setup acculmalting variables
    float accumalteDensity = 0;
    float transmittance = 1;
    vec3 lightEnergy = vec3(0);
    
    //step along the ray 
    for(int i = 0; i < MaxSamples; i++){

        //checks current ray position is within bounds
        if(dot(sign(rayPoint-textureMin),sign(textureMax-rayPoint)) < 3.0)
        {
            break;
        }
        
        //get density at ray point
        float pointDensity = texture(VolumeTexture, rayPoint).r;
        
        //if any density at current sample, calculate for shadow
        if(pointDensity > 0.001){
            //track another ray from current sample towards light
            vec3 lightRayPoint = rayPoint;
            float shadowDistance = 0;

            //step ray through grid
            for(int j = 0; j < 100; j++){

                //if ray leaves grid or reaches threshold
                if(dot(sign(rayPoint-textureMin),sign(textureMax-rayPoint)) < 3.0
                    || shadowDistance > shadowThreshold)
                {
                    break;
                }
                
                //step ray and sample texture
                lightRayPoint += lightRay;
                float shadowSample = texture(VolumeTexture, lightRayPoint).r;

                //add the density to distance travelled in shadow
                shadowDistance += shadowSample;
            }

            //add the sampled density to the running total
            accumalteDensity += clamp(pointDensity * densityMulti, 0, 1);

            //determine the current sample's shadow
            float shadowTerm = exp(-shadowDistance * shadowDensityMulti);

            //how much light was absorbed
            vec3 absorbed = vec3( accumalteDensity * shadowTerm);
            lightEnergy += absorbed * transmittance;

            //transmittance calculation
            transmittance *= 1 - accumalteDensity;

            //ambient lighting, estimate light scattered from three points on the ray
            shadowDistance = 0;
            shadowDistance += texture(VolumeTexture, rayPoint + vec3(0,0,0.05)).r;
            shadowDistance += texture(VolumeTexture, rayPoint + vec3(0,0,0.1)).r;
            shadowDistance += texture(VolumeTexture, rayPoint + vec3(0,0,0.2)).r;

            //add ambient light
            lightEnergy += exp(-shadowDistance * AmbientDensityCoef) * accumalteDensity * SkyColour * transmittance;
        }

        //move ray along one step
        rayPoint += stepRay;
    }

    return vec4(lightEnergy, transmittance);
} 