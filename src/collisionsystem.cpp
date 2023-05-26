#include "collisionsystem.h"

void CollisionSystem::resolveCollisions()
{
    for(auto& cr : _collisions)
    {
        if(!cr.rigidBodyA->dynamic)
        {
            continue;
        }
        if(cr.collision.normal.y > 0)
        {
            cr.rigidBodyA->onGround = true;
        }
        /*if(cr.collision.normal.y < 0)
        {
            cr.rigidBodyB->onGround = true;
        }*/
        cr.rigidBodyA->velocity = glm::reflect(cr.rigidBodyA->velocity, cr.collision.normal) * 0.5f;
    }
    _collisions.clear();
}

void CollisionSystem::tick(std::set<ecs::Entity*>& entities)
{
    update(entities, [this, &entities](ecs::Entity& entity, const glm::vec3& position, const Collider& collider){
        collider.geometry->setPosition(position);
        auto rigidBodyA = entity.get<component::RigidBody>();
        auto gravity = entity.get<component::Gravity>();
        
        ecs::Slice<const component::Collider>::forEach(entities, [this, &entity, &collider, &rigidBodyA](ecs::Entity& entityB, const Collider& colliderB) {
            if(&entity == &entityB)
            {
                return;
            }
            static lithium::Collision collision;
            auto rigidBodyB = entityB.get<component::RigidBody>();
            if(colliderB.geometry->test(*collider.geometry, collision))
            {
                CollisionRecord cr{
                    rigidBodyA,
                    rigidBodyB,
                    collision
                };
                _collisions.push_back(cr);
            }
        });
    });
    resolveCollisions();
}