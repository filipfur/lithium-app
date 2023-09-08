#version 330 core

#include <lightning.glsl>
#include lights.glsl

uniform sampler2D u_albedo_map;
uniform sampler2D u_normal_map;
uniform sampler2D u_arm_map;

uniform samplerCube u_irradiance_map;
uniform samplerCube u_prefilter_map;
uniform sampler2D u_brdf_lut;

uniform vec4 u_color;
uniform float u_time;

out vec4 fragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;
in vec3 eyePos;

const float ao = 0.5;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(u_normal_map, texCoord).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(fragPos);
    vec3 Q2  = dFdy(fragPos);
    vec2 st1 = dFdx(texCoord);
    vec2 st2 = dFdy(texCoord);

    vec3 N   = normalize(normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main()
{
    vec3 albedo = vec3(texture(u_albedo_map, texCoord) * u_color);
    //vec3 albedo     = pow(texture(u_albedo_map, texCoord).rgb, vec3(2.2));
    vec3 arm  = texture(u_arm_map, texCoord).rgb;
    float metallic   = arm.z;
    float roughness = arm.y;
    float ao = arm.x;
    //float ao        = texture(u_ao_map, texCoord).r;

    //vec3 N = normalize(normal);
    vec3 N = getNormalFromMap();

    vec3 V = normalize(eyePos - fragPos);
    vec3 R = reflect(-V, N);   

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(u_prefilter_map, R,  roughness * MAX_REFLECTION_LOD).rgb;

    vec3 F0 = vec3(0.04); 
    F0      = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < lights.length(); ++i) 
    {
        vec3 L = normalize(lights[i].position - fragPos);
        vec3 H = normalize(V + L);
        float distance    = length(lights[i].position - fragPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = lights[i].color * attenuation;

        float NDF = DistributionGGX(N, H, roughness);       
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0)  + 0.0001;
        vec3 specular     = numerator / denominator;
  
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