#pragma once

#include <memory>
#include "glsimplecamera.h"
#include "glrenderpipeline.h"
#include "glframebuffer.h"

class Pipeline : public lithium::RenderPipeline
{
public:
    Pipeline(const glm::ivec2& resolution);

    ~Pipeline();

    std::shared_ptr<lithium::SimpleCamera> camera()
    {
        return _camera;
    }

    void addFacade(lithium::Renderable* renderable)
    {
        _facadeGroup->pushBack(renderable);
    }

    void addSecondary(lithium::Renderable* renderable)
    {
        _secondaryGroup->pushBack(renderable);
    }

    void setStencilDebug(float value)
    {
        _stencilDebug = value;
    }

private:
    std::shared_ptr<lithium::ShaderProgram> _blockShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _msaaShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _screenShader{nullptr};
    std::shared_ptr<lithium::SimpleCamera> _camera{nullptr};

    std::shared_ptr<lithium::RenderGroup> _mainGroup;
    std::shared_ptr<lithium::RenderGroup> _facadeGroup;
    std::shared_ptr<lithium::RenderGroup> _secondaryGroup;
    std::shared_ptr<lithium::RenderStage> _mainStage;
    std::shared_ptr<lithium::RenderStage> _finalStage;

    std::shared_ptr<lithium::FrameBuffer> _frameBuffer;

    float _stencilDebug{0.0f};
};