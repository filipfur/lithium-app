#pragma once

#include "glapplication.h"
#include "pipeline.h"
#include "glmesh.h"
#include "playercontroller.h"
#include "glgamestate.h"
#include "ecssystem.h"

struct Time
{
    float time{0.0f};
    float deltaTime{0.0f};
};

struct RigidBody
{
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};
    bool onGround{true};
};

struct Jump
{
    enum class State { Idle, Jumping, Falling } state{State::Idle};
    float jumpDuration{0.0f};
    bool hasDouble{true};
};

namespace Component
{
    using Time = ecs::Component<Time, 0, true>;
    using RigidBody = ecs::Component<RigidBody>;
    using Jump = ecs::Component<Jump>;
}

class App : public lithium::Application
{
public:
    App();

    virtual ~App() noexcept;

    virtual void update(float dt) override;

    virtual void onWindowSizeChanged(int width, int height) override;

protected:
    virtual void onFpsCount(int fps) override
    {
        //printf("FPS: %d\n", fps);
    }

private:
    std::shared_ptr<Pipeline> _pipeline{nullptr};
    std::unordered_map<ecs::Entity*,std::shared_ptr<lithium::Object>> _entityObjects;
    std::shared_ptr<lithium::Object> _background;
    float _cameraYaw{glm::pi<float>() * 0.5f};
    float _cameraPitch{glm::pi<float>() * 0.25f};
    glm::vec3 _cameraTarget{0.0f};
    std::shared_ptr<lithium::Input::KeyCache> _keyCache;
    PlayerController _playerController;
    std::vector<lithium::GameState> _gameStates;
    ecs::Entity _playerEntity;
    std::set<ecs::Entity*> _entities;
    
};
