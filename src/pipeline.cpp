#include "pipeline.h"
#include "shape.h"

Pipeline::Pipeline(const glm::ivec2& resolution) : lithium::RenderPipeline{resolution},
    _camera{new lithium::SimpleCamera(glm::perspective(glm::radians(45.0f), (float)resolution.x / (float)resolution.y, 0.01f, 100.0f))},
    _frameBuffer{std::make_shared<lithium::FrameBuffer>(resolution)}
{
    enableDepthTesting();
    enableBlending();
    //enableFaceCulling();
    blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    enableMultisampling();

    _blockShader = std::make_shared<lithium::ShaderProgram>("shaders/object.vert", "shaders/object.frag");
    _blockShader->setUniform("u_texture_0", 0);
    _blockShader->setUniform("u_projection", _camera->projection());

    _screenShader = std::make_shared<lithium::ShaderProgram>("shaders/screenshader.vert", "shaders/screenshader.frag");

    _msaaShader = std::make_shared<lithium::ShaderProgram>("shaders/screenshader.vert", "shaders/msaa.frag");
    _msaaShader->setUniform("u_texture", 0);

    _passthruGeometryShader = std::make_shared<lithium::ShaderProgram>(
        "shaders/geometry.vert", "shaders/object.frag", "shaders/object.geom");
    _passthruGeometryShader->setUniform("u_texture_0", 0);
    _passthruGeometryShader->setUniform("u_projection", _camera->projection());

    _bendGeometryShader = std::make_shared<lithium::ShaderProgram>(
        "shaders/bend.vert", "shaders/object.frag", "shaders/bend.geom");
    _bendGeometryShader->setUniform("u_texture_0", 0);
    _bendGeometryShader->setUniform("u_projection", _camera->projection());

    _explodeGeometryShader = std::make_shared<lithium::ShaderProgram>(
        "shaders/geometry.vert", "shaders/object.frag", "shaders/explode.geom");
    _explodeGeometryShader->setUniform("u_texture_0", 0);
    _explodeGeometryShader->setUniform("u_projection", _camera->projection());

    _normalsGeometryShader = std::make_shared<lithium::ShaderProgram>(
        "shaders/geometry.vert", "shaders/color.frag", "shaders/normals.geom");
    _normalsGeometryShader->setUniform("u_texture_0", 0);
    _normalsGeometryShader->setUniform("u_projection", _camera->projection());

    _camera->setPosition(glm::vec3{3.0f, 3.0f, 3.0f});
    _camera->setTarget(glm::vec3{0.0f});

    _frameBuffer->bind();
    _frameBuffer->createTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_TEXTURE_2D_MULTISAMPLE);
    _frameBuffer->createTexture(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8, GL_DEPTH24_STENCIL8, GL_FLOAT, GL_TEXTURE_2D_MULTISAMPLE);
    _frameBuffer->declareBuffers();
    _frameBuffer->unbind();

    _screenMesh = std::shared_ptr<lithium::Mesh>(shape::Plane());

    _explodeGroup = createRenderGroup([this](lithium::Renderable* renderable) -> bool {
        return renderable->groupId() == 1337;
    });

    _bendGroup = createRenderGroup([this](lithium::Renderable* renderable) {
        return renderable->groupId() == 2;
    });

    _mainGroup = createRenderGroup([this](lithium::Renderable* renderable) -> bool {
        return !renderable->hasAttachments();
    });
    _mainStage = addRenderStage(std::make_shared<lithium::RenderStage>(_frameBuffer, glm::ivec4{0, 0, resolution.x, resolution.y}, [this](){
        clearColor(0.0f, 0.0f, 0.0f, 1.0f);
        clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        _screenShader->use();
        _screenMesh->bind();
        glActiveTexture(GL_TEXTURE0);
        disableDepthWriting();
        _screenMesh->draw();
        enableDepthWriting();

        _blockShader->setUniform("u_view", _camera->view());
        _mainGroup->render(_blockShader);

        _bendGeometryShader->setUniform("u_view", _camera->view());
        _bendGroup->render(_bendGeometryShader);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(4.0f);
        _normalsGeometryShader->setUniform("u_view", _camera->view());
        _mainGroup->render(_normalsGeometryShader);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        _explodeGeometryShader->setUniform("u_view", _camera->view());
        _explodeGroup->render(_explodeGeometryShader);
    }));

    _finalStage = addRenderStage(std::make_shared<lithium::RenderStage>(nullptr, glm::ivec4{0, 0, resolution.x, resolution.y}, [this](){
        clearColor(0.0f, 0.0f, 0.0f, 0.0f);
        clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        _msaaShader->use();
        _mainStage->frameBuffer()->texture(GL_COLOR_ATTACHMENT0)->bind(GL_TEXTURE0);
        _mainStage->frameBuffer()->bindTexture(GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE1);
        _screenMesh->bind();
        _screenMesh->draw();
    }));

    setViewportToResolution();
}

Pipeline::~Pipeline()
{
    //_blockShader = nullptr;
    _passthruGeometryShader = nullptr;
    _bendGeometryShader = nullptr;
    _explodeGeometryShader = nullptr;
    _screenShader = nullptr;
    _msaaShader = nullptr;
    _screenMesh = nullptr;
}