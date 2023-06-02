#pragma once

#include <memory>
#include "glsimplecamera.h"
#include "glrenderpipeline.h"
#include "glframebuffer.h"

class Pipeline : public lithium::RenderPipeline
{
public:
    Pipeline(const glm::ivec2& resolution);

    ~Pipeline() noexcept;

    std::shared_ptr<lithium::SimpleCamera> camera()
    {
        return _camera;
    }

private:
    std::shared_ptr<lithium::ShaderProgram> _blockShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _msaaShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _screenShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _passthruGeometryShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _bendGeometryShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _foldShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _explodeGeometryShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _normalsGeometryShader{nullptr};
    std::shared_ptr<lithium::SimpleCamera> _camera{nullptr};

    std::shared_ptr<lithium::RenderGroup> _explodeGroup;
    std::shared_ptr<lithium::RenderGroup> _bendGroup;
    std::shared_ptr<lithium::RenderGroup> _mainGroup;
    std::shared_ptr<lithium::RenderStage> _mainStage;
    std::shared_ptr<lithium::RenderStage> _finalStage;

    std::shared_ptr<lithium::FrameBuffer> _frameBuffer;
    
    std::shared_ptr<lithium::Mesh> _screenMesh;
};