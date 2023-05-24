#include "shape.h"

lithium::Mesh* shape::Plane()
{
    return new lithium::Mesh(
        std::vector<attr>{ POSITION, NORMAL, UV },
        std::vector<GLfloat>{
            -1.0, -1.0, 0.0f, 	0.0f, 1.0f, 0.0f,	0.0f, 0.0f,
            -1.0,  1.0, 0.0f, 	0.0f, 1.0f, 0.0f,	0.0f, 1.0, 
            1.0,  1.0, 0.0f,	0.0f, 1.0f, 0.0f,	1.0, 1.0,  
            1.0, -1.0, 0.0f, 	0.0f, 1.0f, 0.0f,	1.0, 0.0f
        },
        std::vector<GLuint>{
            0, 2, 1,
            0, 3, 2
        });
}