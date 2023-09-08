#pragma once

#include "glapplication.h"
#include "pipeline.h"
#include "glmesh.h"
#include "glcubemaphdr.h"

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
        printf("FPS: %d\n", fps);
    }

private:
    std::shared_ptr<Pipeline> _pipeline{nullptr};
    std::vector<std::shared_ptr<lithium::Object>> _objects;
    std::shared_ptr<lithium::Object> _skybox{nullptr};
    std::shared_ptr<lithium::CubemapHDR> _cubemapHDR;
    float _cameraAngle{0.0f};
    float _camY{4.0f};
    const float _cameraRadius{4.0f};
    const glm::vec3 _cameraTarget{0.0f, 1.0f, 2.5f};
    int _hoveredTileId{0};
    float _lod{0.0f};
    std::shared_ptr<lithium::Input::KeyCache> _keyCache;
};
