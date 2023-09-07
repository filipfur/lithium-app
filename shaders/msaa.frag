#version 330 core

out vec4 fragColor;

in vec2 texCoord;
uniform sampler2DMS u_texture;
uniform ivec2 u_resolution;

const ivec2 offsets[9] = ivec2[](
    ivec2(-1, 1), ivec2(0, 1), ivec2(1, 1),
    ivec2(-1, 0), ivec2(0, 0), ivec2(1, 0),
    ivec2(-1,-1), ivec2(0,-1), ivec2(1,-1)
);

const float kernel[9] = float[](
    1.0/16.0, 2.0/16.0, 1.0/16.0,
    2.0/16.0, 4.0/16.0, 2.0/16.0,
    1.0/16.0, 2.0/16.0, 1.0/16.0
);

void main()
{
    vec4 diffuse = vec4(0.0);
    for(int i=0; i < 4; i++)
    {
        diffuse += texelFetch(u_texture, ivec2(texCoord * u_resolution), i);
        /*for(int j=0; j < 9; j++)
        {
            diffuse += texelFetch(u_texture, ivec2(texCoord * u_resolution) + offsets[j], i) * kernel[j];
        }*/
    }
    fragColor = diffuse * 0.25;//diffuse * 0.25f;
}