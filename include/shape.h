#pragma once

#include "glmesh.h"

namespace
{
    using attr = lithium::VertexArrayBuffer::AttributeType;
    static constexpr attr POSITION{attr::VEC3};
    static constexpr attr NORMAL{attr::VEC3};
    static constexpr attr UV{attr::VEC2};
    static constexpr attr BONE_IDS{attr::VEC4};
    static constexpr attr BONE_WEIGHTS{attr::VEC4};
    static constexpr attr COLOR{attr::VEC3};
}

namespace shape
{
    lithium::Mesh* Plane();
}
