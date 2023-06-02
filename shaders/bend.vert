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

void main()
{
    mat3 viewRotation = mat3(u_view);
    vec3 r = vec3(1.0, 0.0, 0.0);
    vec3 u = vec3(0.0, 1.0, 0.0);
    vs_out.right = viewRotation * r;
    vs_out.up = viewRotation * u;
    vs_out.rxu = viewRotation * cross(r, u);

    vs_out.fragPos = vec3(u_model * vec4(aPos, 1.0));
    mat3 normalMatrix = mat3(transpose(inverse(u_view * u_model)));
    vs_out.normal = normalize(vec3(vec4(normalMatrix * aNormal, 0.0)));
    vs_out.texCoord = aTexCoords;
    gl_Position = u_view * u_model * vec4(aPos, 1.0); 
}