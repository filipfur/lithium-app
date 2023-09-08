#version 330 core

out vec4 fragColor;

in vec3 texCoord;

uniform samplerCube skybox;

uniform float u_lod;

void main()
{    
    //fragColor = texture(skybox, texCoord);
    
    vec3 envColor = texture(skybox, texCoord).rgb;
    
    envColor = envColor / (envColor + vec3(1.0));
    if(isnan(envColor.x))
        envColor = vec3(1.0, 0.0, 0.0);
    
    envColor = pow(envColor, vec3(1.0/2.2)); 
  
    fragColor = vec4(envColor, 1.0);
}