#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 texCoord;

layout (std140) uniform CameraUBO
{
    mat4 u_projection;
    mat4 u_view;
    vec3 u_eye_pos;
};

void main()
{
    texCoord = aPos;
    mat4 modelView = u_view;
    mat4 scaleMat = mat4(40.0);
    scaleMat[3][3] = 1.0;
    modelView[3][0] = 0.0;
    modelView[3][1] = 0.0;
    modelView[3][2] = 0.0;
    gl_Position = u_projection * modelView * scaleMat * vec4(aPos, 1.0);
} 