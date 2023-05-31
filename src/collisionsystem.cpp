#include "collisionsystem.h"

void CollisionSystem::tick(std::set<ecs::Entity*>& entities)
{
    update(entities, [this, &entities](ecs::Entity& entity, const glm::vec3& position, const Collider& collider){
        collider.geometry->setPosition(position);
        auto rigidBodyA = entity.get<component::RigidBody>();
        auto gravity = entity.get<component::Gravity>();
        
        ecs::Slice<const component::Collider>::forEach(entities, [this, &entity, &collider, &rigidBodyA](ecs::Entity& entityB, const Collider& colliderB) {
            if(&entity == &entityB || entity.id() > entityB.id())
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

void CollisionSystem::resolveCollisions()
{
    for(auto& cr : _collisions)
    {
        if(cr.collision.normal.y > 0)
        {
            cr.rigidBodyA->onGround = true;
        }
        else if(cr.collision.normal.y < 0)
        {
            cr.rigidBodyB->onGround = true;
        }
        /*if(cr.collision.normal.y < 0)
        {
            cr.rigidBodyB->onGround = true;
        }*/
        glm::vec3 relativeVelocity = cr.rigidBodyA->velocity - cr.rigidBodyB->velocity;
            
        float velocityAlongNormal = glm::dot(relativeVelocity, cr.collision.normal);

        float e = std::min(cr.rigidBodyA->restitution, cr.rigidBodyB->restitution);

        float invMass = (cr.rigidBodyA->dynamic ? cr.rigidBodyA->inverseMass : 0.0f)
            + (cr.rigidBodyB->dynamic ? cr.rigidBodyB->inverseMass : 0.0f);

        float j = -(1 + e) * velocityAlongNormal;
        j /= invMass;

        glm::vec3 impulse = j * cr.collision.normal;
        if(cr.rigidBodyA->dynamic)
        {
            cr.rigidBodyA->correction = cr.collision.normal * (cr.rigidBodyA->inverseMass / invMass) * cr.collision.penetrationDepth;
            cr.rigidBodyA->velocity += cr.rigidBodyA->inverseMass * impulse;
        }
        if(cr.rigidBodyB->dynamic)
        {
            cr.rigidBodyB->correction = -cr.collision.normal * (cr.rigidBodyB->inverseMass / invMass) * cr.collision.penetrationDepth;
            cr.rigidBodyB->velocity -= cr.rigidBodyB->inverseMass * impulse;
        }
        //cr.rigidBodyA->velocity = glm::reflect(cr.rigidBodyA->velocity, cr.collision.normal) * 0.5f;
    }
    _collisions.clear();
}