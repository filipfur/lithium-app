#version 330 core

out vec4 fragColor;

in vec2 texCoord;
in vec3 normal;
in vec3 color;
uniform sampler2DMS u_texture;
uniform sampler2DMS u_depth_stencil;
uniform float u_stencil_debug;

float near_plane = 0.1;
float far_plane = 100.0;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

void main()
{
    vec4 diffuse = vec4(0.0);
    for(int i=0; i < 4; i++)
    {
        diffuse += texelFetch(u_texture, ivec2(gl_FragCoord), i);
    }
    diffuse *= 0.25;
    vec4 depthStencil = vec4(0.0);
    for(int i=0; i < 4; i++)
    {
        depthStencil += texelFetch(u_depth_stencil, ivec2(gl_FragCoord), i);
    }
    depthStencil *= 0.25;
    //fragColor = vec4(mix(diffuse.rgb, depthStencil.r * vec3(1, 0, 0), u_stencil_debug), 1.0);
    float linearDepth = 1.0 - LinearizeDepth(depthStencil.r) / far_plane;
    fragColor = vec4(diffuse.rgb * linearDepth, 1.0);
}