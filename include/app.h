#pragma once

#include "glapplication.h"
#include "pipeline.h"
#include "glmesh.h"
#include "glcanvas.h"

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
    float _cameraAngle{0.0f};
    std::shared_ptr<lithium::Input::KeyCache> _keyCache;
    std::shared_ptr<lithium::Canvas> _canvas;
    glm::vec2 _dragPosition{0.0f};
};
