#pragma once

#include <set>
#include "glapplication.h"
#include "pipeline.h"
#include "glmesh.h"
#include "glentity.h"
#include "ecssystem.h"
#include "component.h"
#include "collisionsystem.h"

class App : public lithium::Application
{
public:
    App();

    virtual ~App() noexcept;

    virtual void update(float dt) override;

    virtual void onFramebufferResized(int width, int height);

private:
    std::shared_ptr<Pipeline> _pipeline{nullptr};
    std::set<ecs::Entity*> _entities;
    float _cameraAngle{glm::pi<float>() * 0.25f};
    std::shared_ptr<lithium::Input::KeyCache> _keyCache;

    ecs::System<const component::Time, const component::Gravity, component::RigidBody> _gravitySystem;

    ecs::System<const component::Time, component::Force, component::RigidBody> _forceSystem;

    ecs::System<const component::Time, component::RigidBody, lithium::Entity::Translation> _physicsSystem;

    ecs::System<const lithium::Entity::Translation,
        const lithium::Entity::Rotation,
        const lithium::Entity::Scale,
        lithium::Entity::ModelMatrix> _transformationSystem;
    
    CollisionSystem _collisionSystem;
    std::vector<std::shared_ptr<lithium::Entity>> _objects;
};
