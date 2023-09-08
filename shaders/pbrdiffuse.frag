#version 330 core

#include <lightning.glsl>
#include lights.glsl

uniform vec4 u_color;
uniform float u_time;
uniform float u_roughness;
uniform float u_metallic;

uniform sampler2D u_diffuse_map;
uniform sampler2D u_ao_map;
uniform samplerCube u_irradiance_map;
uniform samplerCube u_prefilter_map;
uniform sampler2D u_brdf_lut;

out vec4 fragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;
in vec3 eyePos;

const float ao = 1.0;

void main()
{
    vec3 albedo = vec3(u_color * texture(u_diffuse_map, texCoord));
    //float ao = texture(u_ao_map, texCoord).r;
    float metallic  = u_metallic;
    float roughness = u_roughness;

    vec3 N = normalize(normal);
    vec3 V = normalize(eyePos - fragPos);
    vec3 R = reflect(-V, N);   

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(u_prefilter_map, R,  roughness * MAX_REFLECTION_LOD).rgb;

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < lights.length(); ++i) 
    {
        vec3 L = normalize(lights[i].position - fragPos);
        vec3 H = normalize(V + L);
        float distance = length(lights[i].position - fragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lights[i].color * attenuation;

        float NDF = DistributionGGX(N, H, roughness);       
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0)  + 0.0001;
        vec3 specular = numerator / denominator;
  
        float NdotL = max(dot(N, L), 0.0);        
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    //vec3 ambient = vec3(0.03) * albedo * ao;
    
    vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kD = 1.0 - kS;
    vec3 irradiance = texture(u_irradiance_map, N).rgb;
    vec3 diffuse    = irradiance * albedo;

    vec2 envBRDF  = texture(u_brdf_lut, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (kS * envBRDF.x + envBRDF.y);

    vec3 ambient    = (kD * diffuse + specular) * ao;

    vec3 color   = ambient + Lo;

    color = color / (color + vec3(1.0));
    fragColor = vec4(pow(color, vec3(1.0/2.2)), 1.0);
}