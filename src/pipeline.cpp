#include "pipeline.h"
#include "glplane.h"
#include <random>

Pipeline::Pipeline(const glm::ivec2& resolution) : lithium::RenderPipeline{resolution},
    _camera{new lithium::SimpleCamera(glm::perspective(glm::radians(45.0f), (float)resolution.x / (float)resolution.y, 0.1f, 100.0f))},
    _frameBuffer{std::make_shared<lithium::FrameBuffer>(resolution)},
    _ssaoBuffer{std::make_shared<lithium::FrameBuffer>(resolution)},
    _blurBuffer{std::make_shared<lithium::FrameBuffer>(resolution)},
    _gBuffer{std::make_shared<lithium::FrameBuffer>(resolution)}
{
    enableDepthTesting();
    enableBlending();
    blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    enableFaceCulling();
    enableMultisampling();

    _blockShader = std::make_shared<lithium::ShaderProgram>("shaders/object.vert", "shaders/object.frag");
    _blockShader->setUniform("u_texture_0", 0);
    _blockShader->setUniform("u_projection", _camera->projection());

    _gShader = std::make_shared<lithium::ShaderProgram>("shaders/object.vert", "shaders/gshader.frag");
    _gShader->setUniform("u_texture_0", 0);
    _gShader->setUniform("u_projection", _camera->projection());

    _screenShader = std::make_shared<lithium::ShaderProgram>("shaders/screenshader.vert", "shaders/screenshader.frag");
    _screenShader->setUniform("gPosition", 0);
    _screenShader->setUniform("gNormal", 1);
    _screenShader->setUniform("gAlbedo", 2);
    _screenShader->setUniform("gOcclusion", 3);
    _screenShader->setUniform("projection", _camera->projection(), true);

    _ssaoShader = std::make_shared<lithium::ShaderProgram>("shaders/screenshader.vert", "shaders/ssao.frag");
    _ssaoShader->setUniform("gPosition", 0);
    _ssaoShader->setUniform("gNormal", 1);
    _ssaoShader->setUniform("texNoise", 3);
    _ssaoShader->setUniform("projection", _camera->projection(), true);
    const std::function<float(float,float,float)> ourLerp = [](float a, float b, float f) -> float {
        return a + f * (b - a);
    };
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
    for (unsigned int i = 0; i < 128; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 128.0f;

        // scale samples s.t. they're more aligned to center of kernel
        scale = ourLerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }
    _ssaoShader->setUniform("samples", ssaoKernel);

    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 64; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
    _noiseTexture = std::make_shared<lithium::Texture<float>>((float*)ssaoNoise.data(), 8, 8, GL_FLOAT, GL_RGBA32F, GL_RGB, GL_TEXTURE_2D);
    _noiseTexture->setFilter(GL_NEAREST);
    _noiseTexture->setWrap(GL_REPEAT);

    _msaaShader = std::make_shared<lithium::ShaderProgram>("shaders/screenshader.vert", "shaders/msaa.frag");
    _msaaShader->setUniform("u_texture", 0);
    _msaaShader->setUniform("u_resolution", resolution);

    _ssaoBuffer->bind();
    _ssaoBuffer->createTexture(GL_COLOR_ATTACHMENT0, GL_RED, GL_RED, GL_FLOAT, GL_TEXTURE_2D)->setFilter(GL_NEAREST);
    //_ssaoBuffer->createRenderBuffer(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, false);
    //_ssaoBuffer->declareBuffers();
    _ssaoBuffer->unbind();

    _blurBuffer->bind();
    _blurBuffer->createTexture(GL_COLOR_ATTACHMENT0, GL_RED, GL_RED, GL_FLOAT, GL_TEXTURE_2D)->setFilter(GL_NEAREST);
    _blurBuffer->unbind();

    _gBuffer->bind();
    _gBuffer->createTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_TEXTURE_2D)->setFilter(GL_NEAREST); // Position
    _gBuffer->createTexture(GL_COLOR_ATTACHMENT1, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_TEXTURE_2D)->setFilter(GL_NEAREST); // Normal
    _gBuffer->createTexture(GL_COLOR_ATTACHMENT2, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, GL_TEXTURE_2D)->setFilter(GL_NEAREST); // Albedo
    _gBuffer->createRenderBuffer(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, false);
    _gBuffer->declareBuffers();
    _gBuffer->unbind();

    _frameBuffer->bind();
    _frameBuffer->createTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_TEXTURE_2D_MULTISAMPLE);
    //_frameBuffer->createTexture(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8, GL_DEPTH24_STENCIL8, GL_FLOAT, GL_TEXTURE_2D_MULTISAMPLE);
    //s_frameBuffer->declareBuffers();
    _frameBuffer->unbind();

    _screenMesh = std::shared_ptr<lithium::Mesh>(lithium::Plane2D());

    _screenGroup = createRenderGroup([this](lithium::Renderable* renderable) -> bool {
        return renderable->groupId() == BACKGROUND;
    });

    _mainGroup = createRenderGroup([this](lithium::Renderable* renderable) -> bool {
        return dynamic_cast<lithium::Object*>(renderable) && !renderable->hasAttachments();
    });

    clearColor(0.0f, 0.0f, 0.0f, 1.0f);
    setViewportToResolution();

    _gStage = addRenderStage(std::make_shared<lithium::RenderStage>(_gBuffer, [this](){
        clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        _gShader->setUniform("u_view", _camera->view());
        _mainGroup->render(_gShader.get());
    }));

    _ssaoStage = addRenderStage(std::make_shared<lithium::RenderStage>(_ssaoBuffer, [this](){
        clear(GL_COLOR_BUFFER_BIT);
        _gStage->frameBuffer()->texture(GL_COLOR_ATTACHMENT0)->bind(GL_TEXTURE0);
        _gStage->frameBuffer()->texture(GL_COLOR_ATTACHMENT1)->bind(GL_TEXTURE1);
        _gStage->frameBuffer()->texture(GL_COLOR_ATTACHMENT2)->bind(GL_TEXTURE2);
        _noiseTexture->bind(GL_TEXTURE3);

        //disableDepthWriting();
        _screenGroup->render(_ssaoShader.get());
        //enableDepthWriting();
    }));

    _blurStage = std::make_shared<lithium::ExBlurStage>(_blurBuffer,
        _ssaoStage->frameBuffer()->texture(GL_COLOR_ATTACHMENT0));
    addRenderStage(_blurStage);

    _mainStage = addRenderStage(std::make_shared<lithium::RenderStage>(_frameBuffer, [this](){
        clear(GL_COLOR_BUFFER_BIT);
        //_gStage->frameBuffer()->texture(GL_COLOR_ATTACHMENT0)->bind(GL_TEXTURE0);
        //_gStage->frameBuffer()->texture(GL_COLOR_ATTACHMENT1)->bind(GL_TEXTURE1);
        //_gStage->frameBuffer()->texture(GL_COLOR_ATTACHMENT2)->bind(GL_TEXTURE2);
        _blurStage->frameBuffer()->texture(GL_COLOR_ATTACHMENT0)->bind(GL_TEXTURE3);
        //disableDepthWriting();
        _screenShader->setUniform("u_time", _time);
        _screenGroup->render(_screenShader.get());
        //enableDepthWriting();
    }));

    _finalStage = addRenderStage(std::make_shared<lithium::RenderStage>(nullptr, [this](){
        clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        _msaaShader->use();
        _mainStage->frameBuffer()->texture(GL_COLOR_ATTACHMENT0)->bind(GL_TEXTURE0);
        //_mainStage->frameBuffer()->bindTexture(GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE1);
        _screenMesh->bind();
        _screenMesh->draw();
    }));
}

void Pipeline::setResolution(const glm::ivec2& resolution)
{
    lithium::RenderPipeline::setResolution(resolution);
    //_msaaShader->setUniform("u_resolution", resolution);
    //_camera->setProjection(glm::perspective(glm::radians(45.0f), (float)resolution.x / (float)resolution.y, 0.1f, 100.0f));
    //_blockShader->setUniform("u_projection", _camera->projection());
}

Pipeline::~Pipeline()
{
    _blockShader = nullptr;
    _screenShader = nullptr;
    _msaaShader = nullptr;
    _screenMesh = nullptr;
}