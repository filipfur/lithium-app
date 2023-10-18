#version 330 core

layout (std140) uniform SceneBlock
{
    vec4 u_point_lights[10];
};

uniform sampler2D u_texture_0;
uniform vec4 u_color;
uniform float u_time;
uniform vec3 u_view_pos;

out vec4 fragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 fragPos;

void main()
{
    vec3 color = texture(u_texture_0, texCoord).rgb * abs(normal);
    fragColor = vec4(pow(color, vec3(1.0/2.2)), 1.0);
}