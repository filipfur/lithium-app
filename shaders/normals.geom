#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 2) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

const float MAGNITUDE = 0.5;
  
uniform mat4 u_projection;

void GenerateLine(int index)
{
    gl_Position = u_projection * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = u_projection * (gl_in[index].gl_Position + 
                                vec4(gs_in[index].normal, 0.0) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    /*GenerateLine(0); // first vertex normal
    GenerateLine(1); // second vertex normal
    GenerateLine(2); // third vertex normal*/

    vec3 center = (gl_in[0].gl_Position.xyz + gl_in[1].gl_Position.xyz + gl_in[2].gl_Position.xyz) / 3.0;

    vec3 toCenter = center - gl_in[0].gl_Position.xyz;

    vec3 a = gl_in[0].gl_Position.xyz - gl_in[1].gl_Position.xyz;
    vec3 b = gl_in[2].gl_Position.xyz - gl_in[1].gl_Position.xyz;

    vec3 direction = normalize(cross(a, b));

    gl_Position = u_projection * (gl_in[0].gl_Position + vec4(toCenter, 0.0));
    EmitVertex();
    gl_Position = u_projection * (gl_in[0].gl_Position + vec4(toCenter + gs_in[0].normal * MAGNITUDE, 0.0));
    EmitVertex();
    EndPrimitive();
}  