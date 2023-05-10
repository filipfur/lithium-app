#version 330 core

out vec4 fragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 color;
uniform sampler2D u_texture;

void main()
{
    vec2 uv = vec2(texCoord.x, 1.0 - texCoord.y);
    fragColor = vec4(texture2D(u_texture, uv).rrr, 1.0);
}