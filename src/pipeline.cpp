#include "pipeline.h"
#include "glplane.h"

Pipeline::Pipeline(const glm::ivec2& resolution) : lithium::RenderPipeline{resolution},
    _camera{new lithium::SimpleCamera(glm::perspective(glm::radians(45.0f), (float)resolution.x / (float)resolution.y, 0.1f, 100.0f))},
    _frameBuffer{std::make_shared<lithium::FrameBuffer>(resolution)},
    _cameraUBO{sizeof(glm::mat4) * 2 + sizeof(glm::vec3), "CameraUBO", 0}
{
    enableDepthTesting();
    enableBlending();
    blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    enableMultisampling();

    _blockShader = std::make_shared<lithium::ShaderProgram>("shaders/object.vert", "shaders/object.frag");
    _blockShader->setUniform("u_texture_0", 0);
    _blockShader->setUniform("u_projection", _camera->projection());
    _screenShader = std::make_shared<lithium::ShaderProgram>("shaders/screen.vert", "shaders/screen.frag");
    _msaaShader = std::make_shared<lithium::ShaderProgram>("shaders/screen.vert", "shaders/msaa.frag");
    _msaaShader->setUniform("u_texture", 0);
    _msaaShader->setUniform("u_resolution", resolution);

    _cameraUBO.bindBufferBase({
        _blockShader.get()});
    _cameraUBO.bufferSubData(0, _camera->projection());

    _frameBuffer->bind();
    _frameBuffer->createTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_TEXTURE_2D_MULTISAMPLE);
    _frameBuffer->createTexture(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8, GL_DEPTH24_STENCIL8, GL_FLOAT, GL_TEXTURE_2D_MULTISAMPLE);
    _frameBuffer->declareBuffers();
    _frameBuffer->unbind();

    _screenMesh = std::shared_ptr<lithium::Mesh>(lithium::Plane2D());

    _screenGroup = createRenderGroup([this](lithium::Renderable* renderable) -> bool {
        return renderable->groupId() == BACKGROUND;
    });

    _mainGroup = createRenderGroup([this](lithium::Renderable* renderable) -> bool {
        return dynamic_cast<lithium::Object*>(renderable) && !renderable->hasAttachments();
    });

    clearColor(0.0f, 0.0f, 0.0f, 1.0f);

    _mainStage = addRenderStage(std::make_shared<lithium::RenderStage>(_frameBuffer, [this](){
        static const GLuint viewOffset{static_cast<GLuint>(sizeof(glm::mat4))};
        static const GLuint eyePosOffset{static_cast<GLuint>(sizeof(glm::mat4) * 2)};
        _cameraUBO.bufferSubData(viewOffset, _camera->view());
        _cameraUBO.bufferSubData(eyePosOffset, _camera->position());

        clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        disableDepthWriting();
        _screenGroup->render(_screenShader.get());
        enableDepthWriting();
        _mainGroup->render(_blockShader.get());
    }));

    _finalStage = addRenderStage(std::make_shared<lithium::RenderStage>(nullptr, [this](){
        setViewportToResolution();
        clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        _msaaShader->use();
        _mainStage->frameBuffer()->texture(GL_COLOR_ATTACHMENT0)->bind(GL_TEXTURE0);
        _mainStage->frameBuffer()->bindTexture(GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE1);
        _screenMesh->bind();
        _screenMesh->draw();
    }));

}

void Pipeline::setResolution(const glm::ivec2& resolution)
{
    lithium::RenderPipeline::setResolution(resolution);
}

Pipeline::~Pipeline() noexcept
{
    _blockShader = nullptr;
    _screenShader = nullptr;
    _msaaShader = nullptr;
    _screenMesh = nullptr;
}