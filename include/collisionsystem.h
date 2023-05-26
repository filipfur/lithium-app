#pragma once

#include <set>

#include "glaabb.h"
#include "glcollision.h"
#include "component.h"
#include "ecssystem.h"
#include "glentity.h"

struct CollisionRecord
{
    RigidBody* rigidBodyA;
    RigidBody* rigidBodyB;
    lithium::Collision collision;
};

class CollisionSystem
    : public ecs::System<const lithium::Entity::Translation, const component::Collider>
{
public:

    void tick(std::set<ecs::Entity*>& entities);

private:

    void resolveCollisions();

    std::vector<CollisionRecord> _collisions; // Store penetation depth seperate
};