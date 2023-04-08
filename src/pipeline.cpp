#include "assetfactory.h"
#include "pipeline.h"

Pipeline::Pipeline(const glm::ivec2& resolution) : lithium::RenderPipeline{resolution},
    _camera{new lithium::SimpleCamera(glm::perspective(glm::radians(45.0f), (float)resolution.x / (float)resolution.y, 0.1f, 100.0f))},
    _frameBuffer{std::make_shared<lithium::FrameBuffer>(resolution)}
{
    enableDepthTesting();
    enableStencilTesting();
    enableBlending();
    blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    enableFaceCulling();
    cullBackFace();
    stencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glEnable(GL_MULTISAMPLE);


    _blockShader = std::make_shared<lithium::ShaderProgram>("shaders/object.vert", "shaders/object.frag");
    _blockShader->setUniform("u_texture_0", 0);
    _blockShader->setUniform("u_projection", _camera->projection());
    _screenShader = std::make_shared<lithium::ShaderProgram>("shaders/screenshader.vert", "shaders/screenshader.frag");
    _msaaShader = std::make_shared<lithium::ShaderProgram>("shaders/screenshader.vert", "shaders/msaa.frag");
    _msaaShader->setUniform("u_texture", 0);
    _msaaShader->setUniform("u_depth_stencil", 1);
    _camera->setPosition(glm::vec3{3.0f, 3.0f, 3.0f});
    _camera->setTarget(glm::vec3{0.0f});

    _frameBuffer->bind();
    _frameBuffer->createTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_TEXTURE_2D_MULTISAMPLE);
    _frameBuffer->createTexture(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8, GL_DEPTH24_STENCIL8, GL_FLOAT, GL_TEXTURE_2D_MULTISAMPLE);
    _frameBuffer->declareBuffers();
    //_frameBuffer->createRenderBuffer(lithium::RenderBuffer::Mode::MULTISAMPLED, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT);
    _frameBuffer->unbind();

    _facadeGroup = createRenderGroup([this](lithium::Renderable* renderable) -> bool {
        return false;
    });

    _secondaryGroup = createRenderGroup([this](lithium::Renderable* renderable) -> bool {
        return false;
    });

    _mainGroup = createRenderGroup([this](lithium::Renderable* renderable) -> bool {
        return dynamic_cast<lithium::Object*>(renderable);
    });

    _mainStage = addRenderStage(std::make_shared<lithium::RenderStage>(_frameBuffer, glm::ivec4{0, 0, resolution.x, resolution.y}, [this](){

        enableStencilWriting();
        stencilFunc(GL_ALWAYS, 0x01); // all fragments should pass the stencil test
        enableStencilWriting();

        clearColor(0.0f, 0.0f, 0.0f, 0.0f);
        clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        _blockShader->setUniform("u_view", _camera->view());
        _blockShader->setUniform("u_time", 0.0f);

        disableColorWriting();
        //disableDepthWriting();
        _facadeGroup->render(_blockShader.get());
        //enableDepthWriting();
        enableColorWriting();

        stencilFunc(GL_ALWAYS, 0x00);
        _mainGroup->render(_blockShader.get());

        stencilFunc(GL_EQUAL, 0x01);
        disableStencilWriting();
        depthFunc(GL_ALWAYS);
        _secondaryGroup->render(_blockShader.get());
        depthFunc(GL_LESS);

        stencilFunc(GL_ALWAYS, 0xFF);
        /*stencilFunc(GL_NOTEQUAL, 0x01, 0xFF);
        _mainGroup->render(_blockShader.get());*/
    }));

    /*_mainStage = addRenderStage(std::make_shared<lithium::RenderStage>(_frameBuffer, glm::ivec4{0, 0, resolution.x, resolution.y}, [this](){

        stencilMask(0xFF);
        stencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        clearColor(0.0f, 0.0f, 0.0f, 0.0f);
        clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        _blockShader->setUniform("u_view", _camera->view());
        _blockShader->setUniform("u_time", 0.0f);
        stencilFunc(GL_ALWAYS, 0x01); // all fragments should pass the stencil test
        enableStencilWriting();

        disableColorWriting();
        disableDepthWriting();
        cullFrontFace();
        _facadeGroup->render(_blockShader.get());
        cullBackFace();
        enableDepthWriting();
        enableColorWriting();

        stencilFunc(GL_ALWAYS, 0x00);
        _facadeGroup->render(_blockShader.get());

        stencilFunc(GL_EQUAL, 0x01);
        disableStencilWriting();
        _secondaryGroup->render(_blockShader.get());

        stencilFunc(GL_NOTEQUAL, 0x01, 0xFF);
        _mainGroup->render(_blockShader.get());
    }));*/

    /*
    With stencil mesh instead.

        enableStencilWriting();
        stencilFunc(GL_ALWAYS, 0x01); // all fragments should pass the stencil test
        stencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        clearColor(0.0f, 0.0f, 0.0f, 0.0f);
        clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        _blockShader->setUniform("u_view", _camera->view());
        _blockShader->setUniform("u_time", 0.0f);

        disableStencilWriting();
        _mainGroup->render(_blockShader.get());

        enableStencilWriting();
        disableColorWriting();
        disableDepthWriting();
        _facadeGroup->render(_blockShader.get());
        enableDepthWriting();
        enableColorWriting();

        stencilFunc(GL_EQUAL, 0x01);
        disableStencilWriting();
        _secondaryGroup->render(_blockShader.get());
        stencilFunc(GL_ALWAYS, 0xFF);

    */

    _finalStage = addRenderStage(std::make_shared<lithium::RenderStage>(nullptr, glm::ivec4{0, 0, resolution.x, resolution.y}, [this](){
        clearColor(0.0f, 0.0f, 0.0f, 0.0f);
        clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        _msaaShader->use();
        _msaaShader->setUniform("u_stencil_debug", _stencilDebug);
        _mainStage->frameBuffer()->texture(GL_COLOR_ATTACHMENT0)->bind(GL_TEXTURE0);
        _mainStage->frameBuffer()->bindTexture(GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE1);
        //_borderDepthFBO->bindTexture(GL_COLOR_ATTACHMENT0, GL_TEXTURE2);
        AssetFactory::getMeshes()->screen->bind();
        AssetFactory::getMeshes()->screen->draw();
    }));

    setViewportToResolution();
}

Pipeline::~Pipeline()
{
    _blockShader = nullptr;
}