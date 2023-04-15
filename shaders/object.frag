#version 460 core

layout (std140) uniform SceneBlock
{
    vec4 u_point_lights[10];
};

uniform sampler2D u_texture_0;
uniform vec4 u_color;
uniform float u_time;
uniform vec3 u_view_pos;

out vec4 fragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

void calcLight(inout float diffuse, inout float specular, inout float fresnel, in vec3 lightPos, in float strength)
{
    vec3 lightDelta = lightPos - fragPos;
    vec3 lightDir = normalize(lightDelta);

    float diff = max(dot(lightDir, normal.xyz), 0.0);

    vec3 viewDir = normalize(u_view_pos - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);  

    float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    float fres = clamp(1.0 - dot(vec3(0,1,0), viewDir), 0.0, 1.0);

    float distance = length(lightDelta * 1.0);
    float attenuation = 1.0 / (distance * distance) * strength;

    diffuse += diff * attenuation;
    specular += spec * attenuation;
    fresnel += fres * attenuation;
}

void main()
{
    
    vec4 color = texture(u_texture_0, texCoord) * u_color;
    vec3 ambient = color.rgb * 0.2;
    float exposure = 1.0;

    float diffuse = 0.0;
    float specular = 0.0;
    float fresnel = 0.0;

    vec4 lightX = vec4(-1.6, -1.6, 1.6, 1.6);
    vec4 lightZ = vec4(1.6, -1.6, 1.6, -1.6);
    vec4 light2X = vec4(1.4, -1.4, 0.0, 0.0);
    vec4 light2Z = vec4(0.0, 0.0, 1.4, -1.4);

    //calcLight(diffuse, specular, fresnel, vec2(0.0, 0.0));
    for(int i=0; i < 4; ++i)
    {
        calcLight(diffuse, specular, fresnel, vec3(light2X[i], 0.5, light2Z[i]), 0.2);
    }
    /*for(int i=0; i < 10; ++i)
    {
        calcLight(diffuse, specular, fresnel, u_point_lights[i].xyz, u_point_lights[i].w);
    }*/

    fragColor = vec4(ambient + (diffuse + specular + fresnel) * color.rgb * vec3(1.0, 0.6, 0.4), color.a);
    fragColor.rgb = vec3(1.0) - exp(-fragColor.rgb * exposure);
    fragColor.rgb = pow(fragColor.rgb, vec3(1.0/2.2));
}