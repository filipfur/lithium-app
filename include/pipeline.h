#pragma once

#include <memory>
#include "glsimplecamera.h"
#include "glrenderpipeline.h"
#include "glframebuffer.h"
#include "gluniformbufferobject.h"

class Pipeline : public lithium::RenderPipeline
{
public:
    enum Group
    {
        DEFAULT,
        SKYBOX,
        PBR,
        PBR_POLY_HAVEN,
        PBR_DIFFUSE,
    };

    Pipeline(const glm::ivec2& resolution);

    ~Pipeline() noexcept;

    std::shared_ptr<lithium::SimpleCamera> camera()
    {
        return _camera;
    }

    void setTime(float time)
    {
        _time = time;
    }

    virtual void setResolution(const glm::ivec2& resolution) override;

private:
    /* Shaders */
    std::shared_ptr<lithium::ShaderProgram> _skyboxShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _pbrShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _pbrPolyHavenShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _pbrBaseColorShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _pbrDiffuseShader{nullptr};
    std::shared_ptr<lithium::ShaderProgram> _msaaShader{nullptr};
    std::shared_ptr<lithium::SimpleCamera> _camera{nullptr};

    /*Render groups*/
    std::shared_ptr<lithium::RenderGroup> _screenGroup;
    std::shared_ptr<lithium::RenderGroup> _pbrGroup;
    std::shared_ptr<lithium::RenderGroup> _pbrDiffuseGroup;
    std::shared_ptr<lithium::RenderGroup> _pbrPolyHavenGroup;
    std::shared_ptr<lithium::RenderGroup> _pbrBaseColorGroup;
    std::shared_ptr<lithium::RenderGroup> _skyboxGroup;

    /*Render stages*/
    std::shared_ptr<lithium::RenderStage> _mainStage;
    std::shared_ptr<lithium::RenderStage> _finalStage;

    /* Framebuffers */
    std::shared_ptr<lithium::FrameBuffer> _frameBuffer;

    std::shared_ptr<lithium::UniformBufferObject> _cameraUBO;
    
    /* Meshes */
    std::shared_ptr<lithium::Mesh> _screenMesh;

    float _time{0.0f};
};