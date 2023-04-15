#version 460 core

out vec4 fragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 color;
uniform sampler2D u_texture;

void main()
{
    fragColor = vec4(mix(vec3(1.0, 0.5, 0.0) * 1.0, vec3(1.0, 0.0, 0.5) * 0.7, (texCoord.y) * 0.75 - 0.25), 1.0);
}