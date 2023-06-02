#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_model;
uniform float u_time;

out VS_OUT {
    vec2 texCoord;
    vec3 normal;
    vec3 fragPos;
} vs_out;

/*out vec2 gs_texCoord;
out vec3 gs_normal;
out vec3 gs_fragPos;*/

void main()
{
    vs_out.texCoord = aTexCoords;
    mat3 normalMatrix = transpose(inverse(mat3(u_model)));
    vs_out.normal = normalize(normalMatrix * aNormal);
    vs_out.fragPos = vec3(u_model * vec4(aPos, 1.0));
    gl_Position = u_projection * u_view * u_model * vec4(aPos, 1.0);
}