#pragma once

#include "glaabb.h"
#include "glcollision.h"
#include "component.h"
#include "ecssystem.h"

class CollisionSystem
    : public ecs::System<const lithium::Entity::Translation, const component::Collider>
{
public:

    void tick(std::set<ecs::Entity*>& entities)
    {
        update(entities, [](ecs::Entity& entity, const glm::vec3& position, const Collider& collider) {
            static lithium::AABB floorBounds{glm::vec3{0.0f, 0.2f, 0.0f}, glm::vec3{-10.0f, -0.1f, -10.0f}, glm::vec3{10.0f, 0.1f, 10.0f}};
            static lithium::Collision collision;
            collider.geometry->setPosition(position);
            auto rigidBody = entity.get<component::RigidBody>();
            auto gravity = entity.get<component::Gravity>();
            if(collider.geometry->intersect(floorBounds, collision))
            {
                rigidBody->velocity = glm::reflect(rigidBody->velocity, collision.normal) * 0.5f;
                if(gravity)
                {
                    gravity->active = false;
                }
            }
            else
            {
                if(glm::length2(rigidBody->velocity) < 0.001f)
                {
                    rigidBody->velocity = glm::vec3{0.0f};
                }
                else
                {
                    if(gravity)
                    {
                        gravity->active = true;
                    }
                }
            }
        });
    }
private:
    std::vector<lithium::Collision> _collision; // Store penetation depth seperate
};