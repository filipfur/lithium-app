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

uniform float u_time;

void main() {


    vec3 a = gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz;
    vec3 b = gl_in[2].gl_Position.xyz - gl_in[1].gl_Position.xyz;

    vec3 direction = normalize(cross(a, b));

    float displacement = (sin(u_time * 8.0) * 0.5 + 0.5) * 0.2;

    for(int i = 0; i < 3; i++)
    {
        texCoord = gs_in[i].texCoord;
        normal = gs_in[i].normal;
        fragPos = gs_in[i].fragPos;
        gl_Position = gl_in[i].gl_Position + vec4(direction * displacement, 0.0); 
        EmitVertex();
    }
    EndPrimitive();
}