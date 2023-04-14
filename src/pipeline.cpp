#include "assetfactory.h"
#include "pipeline.h"
#include "mirror.h"

Pipeline::Pipeline(const glm::ivec2& resolution) : lithium::RenderPipeline{resolution},
    _camera{new lithium::SimpleCamera(glm::perspective(glm::radians(45.0f), (float)resolution.x / (float)resolution.y, 0.1f, 100.0f))},
    _frameBuffer{std::make_shared<lithium::FrameBuffer>(resolution, lithium::FrameBuffer::Mode::MULTISAMPLED)}
{
    enableDepthTesting();
    enableBlending();
    enableStencilTesting();
    enableFaceCulling();
    stencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _blockShader = std::make_shared<lithium::ShaderProgram>("shaders/object.vert", "shaders/object.frag");
    _blockShader->setUniform("u_texture_0", 0);
    _blockShader->setUniform("u_projection", _camera->projection());
    _mirrorShader = std::make_shared<lithium::ShaderProgram>("shaders/object.vert", "shaders/reflectedobject.frag");
    _mirrorShader->setUniform("u_texture_0", 0);
    _mirrorShader->setUniform("u_projection", _camera->projection());
    _screenShader = std::make_shared<lithium::ShaderProgram>("shaders/screenshader.vert", "shaders/screenshader.frag");
    _msaaShader = std::make_shared<lithium::ShaderProgram>("shaders/screenshader.vert", "shaders/msaa.frag");
    _msaaShader->setUniform("u_texture", 0);
    _camera->setPosition(glm::vec3{3.0f, 3.0f, 3.0f});
    _camera->setTarget(glm::vec3{0.0f});

    _frameBuffer->bind();
    _frameBuffer->createTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    _frameBuffer->createTexture(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8, GL_DEPTH24_STENCIL8, GL_FLOAT);
    _frameBuffer->declareBuffers();
    //_frameBuffer->createRenderBuffer(lithium::RenderBuffer::Mode::MULTISAMPLED, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT);
    _frameBuffer->unbind();

    _mainGroup = createRenderGroup([this](lithium::Renderable* renderable) -> bool {
        return dynamic_cast<lithium::Object*>(renderable) && !dynamic_cast<Mirror*>(renderable);
    });

    _mirrorGroup = createRenderGroup([this](lithium::Renderable* renderable) -> bool {
        return dynamic_cast<Mirror*>(renderable);
    });

    _mainStage = addRenderStage(std::make_shared<lithium::RenderStage>(_frameBuffer, glm::ivec4{0, 0, resolution.x, resolution.y}, [this](){

        _blockShader->setUniform("u_view", _camera->view());
        _blockShader->setUniform("u_time", 0.0f);

        enableStencilWriting();
        stencilFunc(GL_ALWAYS, 0x01); // all fragments should pass the stencil test
        enableStencilWriting();

        clearColor(0.0f, 0.0f, 0.0f, 0.0f);
        clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        //_screenShader->use();
        //AssetFactory().getMeshes()->screen->bind();
        //glActiveTexture(GL_TEXTURE0);
        //disableDepthWriting();
        //AssetFactory().getMeshes()->screen->draw();
        //enableDepthWriting();
        disableStencilWriting();
        _mainGroup->render(_blockShader.get());
        
        enableStencilWriting();
        stencilFunc(GL_ALWAYS, 0x02);
        _mirrorGroup->render(_blockShader.get());
        
        disableStencilWriting();
        stencilFunc(GL_EQUAL, 0x02);
        /*_screenShader->use();
        AssetFactory().getMeshes()->screen->bind();
        glActiveTexture(GL_TEXTURE0);
        disableDepthWriting();
        AssetFactory().getMeshes()->screen->draw();
        enableDepthWriting();*/

        glClear(GL_DEPTH_BUFFER_BIT);
        
        _mirrorGroup->forEach([this](lithium::Renderable* renderable){
            auto reflector = dynamic_cast<Mirror*>(renderable);

            glm::vec3 mirrorPosition = reflector->position();
            glm::vec3 mirrorNormal = reflector->reflectionNormal();
            glm::vec3 cameraPosition = _camera->position();
            glm::vec3 cameraToMirror = mirrorPosition - cameraPosition;
            glm::vec3 reflectedCameraToMirror = glm::reflect(cameraToMirror, mirrorNormal);
            glm::vec3 reflectedCamera = mirrorPosition - reflectedCameraToMirror;
            const glm::vec3 eyeToX = _camera->target() - _camera->position();
            glm::vec3 reflectedView = glm::reflect(eyeToX, mirrorNormal);

            //view matrix of the reflected camera
            glm::mat4 view = glm::lookAt(reflectedCamera, reflectedCamera + reflectedView,
                glm::reflect(glm::vec3{0.0f, 1.0f, 0.0f}, mirrorNormal));

            glm::mat4 mirrorMatrix{1.0f};
            mirrorMatrix[0][0] = -1.0f;

            view = mirrorMatrix * view;

            cullFrontFace();
            _mirrorShader->setUniform("u_view", view);
            _mirrorShader->setUniform("u_mirror_pos", mirrorPosition);
            _mainGroup->render(_mirrorShader.get());
            cullBackFace();
        });
        stencilFunc(GL_ALWAYS, 0x01);
    }));

    _finalStage = addRenderStage(std::make_shared<lithium::RenderStage>(nullptr, glm::ivec4{0, 0, resolution.x, resolution.y}, [this](){
        clearColor(0.0f, 0.0f, 0.0f, 1.0f);
        clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        _msaaShader->use();
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