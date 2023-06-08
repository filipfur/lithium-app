#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec2 texCoord;
    vec3 normal;
    vec3 fragPos;
    vec3 up;
    vec3 right;
    vec3 rxu;
} vs_out;

uniform mat4 u_view;
uniform mat4 u_model;
uniform vec3 u_up;
uniform vec3 u_right;

void main()
{
    mat3 viewRotation = mat3(u_view);
    vs_out.right = viewRotation * u_right;
    vs_out.up = viewRotation * u_up;
    vs_out.rxu = viewRotation * cross(u_right, u_up);

    vs_out.fragPos = vec3(u_model * vec4(aPos, 1.0));
    mat3 normalMatrix = mat3(transpose(inverse(u_view * u_model)));
    vs_out.normal = normalize(vec3(vec4(normalMatrix * aNormal, 0.0)));
    vs_out.texCoord = aTexCoords;
    gl_Position = u_view * u_model * vec4(aPos, 1.0); 
}