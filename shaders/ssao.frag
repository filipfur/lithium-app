#version 330 core

out vec4 fragColor;

in vec2 texCoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform vec3 samples[128];
uniform mat4 projection;

const float kernelSize = 128;
const float radius = 0.5;
float bias = 0.025;

const float noiseFactor = 1.0 / 8.0;

const vec2 noiseScale = vec2(1440.0, 800.0) * noiseFactor; 

void main()
{
    vec3 FragPos = texture(gPosition, texCoord).rgb;
    vec3 Normal = texture(gNormal, texCoord).rgb;
    vec3 randomVec = normalize(texture(texNoise, texCoord * noiseScale).xyz);

    vec3 tangent   = normalize(randomVec - Normal * dot(randomVec, Normal));
    vec3 bitangent = cross(Normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, Normal);

    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space
        samplePos = FragPos + samplePos * radius; 
        
        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = texture(gPosition, offset.xy).z; // get depth value of kernel sample
        
        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(FragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck; 
    }

    occlusion = 1.0 - (occlusion / kernelSize);

    //fragColor = vec4(1.0);
    fragColor = vec4(vec3(occlusion), 1.0);
    //fragColor = occlusion;
}