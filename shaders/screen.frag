#version 330 core

out vec4 fragColor;

in vec2 texCoord;
uniform sampler2D u_texture;

void main()
{
    float x = abs(texCoord.y - 0.5);
    fragColor = vec4(0,1.0 - x * 2.0,0,1);
}