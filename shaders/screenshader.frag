#version 460 core

out vec4 fragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 color;
uniform sampler2D u_texture;

void main()
{
    fragColor = texture(u_texture, texCoord);
}