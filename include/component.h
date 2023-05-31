#pragma once

#include "ecscomponent.h"
#include "glgeometry.h"

struct Time
{
    float delta{0.0f};
    float seconds{0.0f};
};

struct Force
{
    std::vector<std::pair<float,glm::vec3>> forces;
};

struct Collider
{
    lithium::Geometry* geometry;
};

struct Gravity
{
    float acceleration{9.81f};
};

struct RigidBody
{
    float mass{1.0f};
    float inverseMass{1.0f};
    glm::vec3 velocity;
    glm::vec3 force;
    glm::vec3 correction{0.0f};
    float drag{0.5f};
    bool dynamic{true};
    bool onGround{false};
    float restitution{0.5f};
};

namespace component
{
    using Time = ecs::Component<Time, 0, true>;
    using Force = ecs::Component<Force>;
    using Collider = ecs::Component<Collider>;
    using Gravity = ecs::Component<Gravity>;
    using RigidBody = ecs::Component<RigidBody>;
}