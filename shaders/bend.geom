#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 8) out;

in VS_OUT {
    vec2 texCoord;
    vec3 normal;
    vec3 fragPos;
    vec3 up;
    vec3 right;
    vec3 rxu;
} gs_in[];

/*in vec2 gs_texCoord[];
in vec3 gs_normal[];
in vec3 gs_fragPos[];*/

out vec2 texCoord;
out vec3 normal;
out vec3 fragPos;

uniform mat4 u_projection;

void createVertex(vec3 offset, vec2 uv)
{
    texCoord = uv;
    normal = gs_in[0].normal;
    fragPos = gs_in[0].fragPos;
    vec3 displacement = offset.x * gs_in[0].right + offset.y * gs_in[0].up + offset.z * gs_in[0].rxu;
    gl_Position = u_projection * (gl_in[0].gl_Position
        + vec4(displacement, 0.0)); 
    EmitVertex();
}

void main()
{
    /*for(int i = 0; i < 3; i++)
    {
        int index = i % 3;
        texCoord = gs_in[0].texCoord;
        normal = gs_in[0].normal;
        fragPos = gs_in[0].fragPos;
        vec4 offset = vec4(0.0);
        if(i == 0)
        {
            offset = vec4(gs_in[0].up * 1.0, 0.0);
        }
        else
        {
            offset = vec4(gs_in[0].right * 1.0 * (i - 1.5), 0.0);
        }
        gl_Position = u_projection * (gl_in[0].gl_Position + offset); 
        EmitVertex();
    }*/
    createVertex(vec3(-1.0, -1.0, 0.0), vec2(0.0, 0.0));
    createVertex(vec3(-1.0, 1.0, 0.0), vec2(0.0, 1.0));
    createVertex(vec3(0.0, -1.0, 0.0), vec2(0.5, 0.0));
    createVertex(vec3(0.0, 1.0, 0.0), vec2(0.5, 1.0));

    createVertex(vec3(0.0, -1.0, 0.0), vec2(0.5, 0.0));
    createVertex(vec3(0.0, 1.0, 0.0), vec2(0.5, 1.0));
    createVertex(vec3(0.0, -1.0, -1.0), vec2(1.0, 0.0));
    createVertex(vec3(0.0, 1.0, -1.0), vec2(1.0, 1.0));
    EndPrimitive();
}