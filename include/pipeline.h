#pragma once

#include <memory>
#include "glsimplecamera.h"
#include "glrenderpipeline.h"
#include "glframebuffer.h"
#include "ex/glblurstage.h"

class Pipeline : public lithium::RenderPipeline
{
public:
    enum Group
    {
        DEFAULT,
        BACKGROUND
    };

    Pipeline(const glm::ivec2& resolution);

    ~Pipeline() noexcept;

    std::shared_ptr<lithium::SimpleCamera> camera()
    {
        return _camera;
    }

    virtual void setResolution(const glm::ivec2& resolution) override;

    void setTime(float time)
    {
        _time = time;
    }

private:
    /* Shaders */
    std::shared_ptr<lithium::ShaderProgram> _gShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _blockShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _msaaShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _screenShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _ssaoShader{nullptr};
    std::shared_ptr<lithium::SimpleCamera> _camera{nullptr};

    /*Render groups*/
    std::shared_ptr<lithium::RenderGroup> _screenGroup;
    std::shared_ptr<lithium::RenderGroup> _mainGroup;

    /*Render stages*/
    std::shared_ptr<lithium::RenderStage> _gStage;
    std::shared_ptr<lithium::RenderStage> _ssaoStage;
    std::shared_ptr<lithium::RenderStage> _mainStage;
    std::shared_ptr<lithium::RenderStage> _finalStage;
    std::shared_ptr<lithium::ExBlurStage> _blurStage;

    /* Framebuffers */
    std::shared_ptr<lithium::FrameBuffer> _gBuffer;
    std::shared_ptr<lithium::FrameBuffer> _ssaoBuffer;
    std::shared_ptr<lithium::FrameBuffer> _frameBuffer;
    std::shared_ptr<lithium::FrameBuffer> _blurBuffer;

    std::shared_ptr<lithium::Texture<float>> _noiseTexture;
    
    /* Meshes */
    std::shared_ptr<lithium::Mesh> _screenMesh;

    float _time{0.0f};
};