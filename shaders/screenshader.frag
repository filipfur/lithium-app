#version 330 core

out vec4 fragColor;

in vec2 texCoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gOcclusion;
uniform float u_time;

const vec3 lightPos = vec3(0.0, 2.0, 2.0);
const vec3 lightColor = vec3(0.92, 0.76, 0.64);

void main()
{
    vec3 FragPos = texture(gPosition, texCoord).rgb;
    vec3 Normal = texture(gNormal, texCoord).rgb;
    vec3 Albedo = texture(gAlbedo, texCoord).rgb;
    float Occlusion = texture(gOcclusion, texCoord).r;

    vec3 ambient = vec3(0.3 * Albedo * Occlusion);
    
    vec3 viewDir = normalize(-FragPos); // viewpos is (0,0,0) in view space
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 diffuse = diff * Albedo * lightColor;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = lightColor * spec;

    float dist = length(lightPos - FragPos);
    float atennuation = 1.0 / (1.0 + 0.09 * dist + 0.032 * dist * dist);

    float stylizedOcclusion = floor(Occlusion * 2.0 + 0.5) / 2.0;
    float stylizedDiffuse = floor(diff * 2.0 + 0.5) / 2.0;

    vec3 lightning = ambient + (diffuse + specular) * atennuation;
    vec3 stylizedLightning = vec3(0.2, 0.2, 0.4) * Albedo * stylizedOcclusion + stylizedDiffuse * Albedo * lightColor;

    fragColor = vec4(pow(mix(stylizedLightning, lightning, mod(int(floor(u_time * 0.25)), 2) ), vec3(1.0/2.2)), 1.0);
}