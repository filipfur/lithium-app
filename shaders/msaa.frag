#version 460 core

out vec4 fragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 color;
uniform sampler2DMS u_texture;

void main()
{
    vec4 diffuse = vec4(0.0);
    for(int i=0; i < 4; i++)
    {
        diffuse += texelFetch(u_texture, ivec2(gl_FragCoord), i);
    }
    fragColor = diffuse * 0.25f;
}