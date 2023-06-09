#pragma once

#include "glapplication.h"
#include "pipeline.h"
#include "glmesh.h"

class App : public lithium::Application
{
public:
    App();

    virtual ~App() noexcept;

    virtual void update(float dt) override;

    virtual void onFramebufferResized(int width, int height);

private:
    std::shared_ptr<Pipeline> _pipeline{nullptr};
    std::vector<std::shared_ptr<lithium::Object>> _objects;
    float _cameraAngle{glm::pi<float>() * 0.25f};
    std::shared_ptr<lithium::Input::KeyCache> _keyCache;
    glm::vec3 _position{0.0f, 0.0f, 0.0f};
    int _rotation{0};
};
