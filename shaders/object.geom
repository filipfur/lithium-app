#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoord;
    vec3 normal;
    vec3 fragPos;
} gs_in[];

/*in vec2 gs_texCoord[];
in vec3 gs_normal[];
in vec3 gs_fragPos[];*/

out vec2 texCoord;
out vec3 normal;
out vec3 fragPos;

void main()
{
    for(int i = 0; i < 3; i++)
    {
        texCoord = gs_in[i].texCoord;
        normal = gs_in[i].normal;
        fragPos = gs_in[i].fragPos;
        gl_Position = gl_in[i].gl_Position; 
        EmitVertex();
    }
    EndPrimitive();
}