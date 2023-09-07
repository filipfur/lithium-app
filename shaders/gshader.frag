#version 330 core

uniform sampler2D u_texture_0;
uniform vec4 u_color;
uniform float u_time;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;

in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;
in vec4 viewPos;

void main()
{
    gPosition = viewPos;
    gNormal = vec4(normalize(normal), 1.0);
    gAlbedo = texture(u_texture_0, texCoord) * u_color;
}