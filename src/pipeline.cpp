#include "pipeline.h"
#include "glplane.h"

Pipeline::Pipeline(const glm::ivec2& resolution) : lithium::RenderPipeline{resolution},
    _camera{new lithium::SimpleCamera(glm::perspective(glm::radians(45.0f), (float)resolution.x / (float)resolution.y, 0.1f, 100.0f))},
    _frameBuffer{std::make_shared<lithium::FrameBuffer>(resolution)},
    _cameraUBO{std::make_shared<lithium::UniformBufferObject>(sizeof(glm::mat4) * 2 + sizeof(glm::vec3), "CameraUBO", 0)}
{
    enableDepthTesting();
    depthFunc(GL_LESS);
    enableBlending();
    blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    enableMultisampling();
    enableFaceCulling();
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    _msaaShader = std::make_shared<lithium::ShaderProgram>("shaders/screen.vert", "shaders/msaa.frag");
    _msaaShader->setUniform("u_texture", 0);
    _msaaShader->setUniform("u_resolution", resolution);

    _skyboxShader = std::make_shared<lithium::ShaderProgram>("shaders/cubemap.vert", "shaders/cubemap.frag");

    _pbrShader = std::make_shared<lithium::ShaderProgram>("shaders/object.vert", "shaders/pbr.frag");
    _pbrShader->setUniform("u_albedo_map", 0);
    _pbrShader->setUniform("u_normal_map", 1);
    _pbrShader->setUniform("u_metallic_map", 2);
    _pbrShader->setUniform("u_roughness_map", 3);
    _pbrShader->setUniform("u_ao_map", 4);
    _pbrShader->setUniform("u_brdf_lut", 7);
    _pbrShader->setUniform("u_irradiance_map", 8);
    _pbrShader->setUniform("u_prefilter_map", 9);

    _pbrPolyHavenShader = std::make_shared<lithium::ShaderProgram>("shaders/object.vert", "shaders/pbrpolyhaven.frag");
    _pbrPolyHavenShader->setUniform("u_albedo_map", 0);
    _pbrPolyHavenShader->setUniform("u_normal_map", 1);
    _pbrPolyHavenShader->setUniform("u_arm_map", 2);
    _pbrPolyHavenShader->setUniform("u_brdf_lut", 7);
    _pbrPolyHavenShader->setUniform("u_irradiance_map", 8);
    _pbrPolyHavenShader->setUniform("u_prefilter_map", 9);

    _pbrBaseColorShader = std::make_shared<lithium::ShaderProgram>("shaders/object.vert", "shaders/pbrbasecolor.frag");
    _pbrBaseColorShader->setUniform("u_brdf_lut", 7);
    _pbrBaseColorShader->setUniform("u_irradiance_map", 8);
    _pbrBaseColorShader->setUniform("u_prefilter_map", 9);

    _pbrDiffuseShader = std::make_shared<lithium::ShaderProgram>("shaders/object.vert", "shaders/pbrdiffuse.frag");
    _pbrDiffuseShader->setUniform("u_diffuse_map", 0);
    _pbrDiffuseShader->setUniform("u_ao_map", 1);
    _pbrDiffuseShader->setUniform("u_brdf_lut", 7);
    _pbrDiffuseShader->setUniform("u_irradiance_map", 8);
    _pbrDiffuseShader->setUniform("u_prefilter_map", 9);

    _frameBuffer->bind();
    _frameBuffer->createTexture(GL_COLOR_ATTACHMENT0, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_TEXTURE_2D_MULTISAMPLE);
    _frameBuffer->createTexture(GL_DEPTH_STENCIL_ATTACHMENT, GL_DEPTH24_STENCIL8, GL_DEPTH24_STENCIL8, GL_FLOAT, GL_TEXTURE_2D_MULTISAMPLE);
    _frameBuffer->declareBuffers();
    _frameBuffer->unbind();

    _cameraUBO->bindBufferBase({
        _skyboxShader.get(),
        _pbrShader.get(),
        _pbrPolyHavenShader.get(),
        _pbrBaseColorShader.get(),
        _pbrDiffuseShader.get()});
    _cameraUBO->bufferSubData(0, _camera->projection());

    _screenMesh = std::shared_ptr<lithium::Mesh>(lithium::Plane2D());

    _skyboxGroup = createRenderGroup([this](lithium::Renderable* renderable) -> bool {
        return renderable->groupId() == SKYBOX;
    });

    _pbrBaseColorGroup = createRenderGroup([this](lithium::Renderable* renderable) -> bool {
        return renderable->groupId() == PBR && !dynamic_cast<lithium::Object*>(renderable)->hasTexture();
    });

    _pbrGroup = createRenderGroup([this](lithium::Renderable* renderable) -> bool {
        return renderable->groupId() == PBR && dynamic_cast<lithium::Object*>(renderable)->hasTexture();
    });

    _pbrDiffuseGroup = createRenderGroup([this](lithium::Renderable* renderable) -> bool {
        return renderable->groupId() == PBR_DIFFUSE;
    });

    _pbrPolyHavenGroup = createRenderGroup([this](lithium::Renderable* renderable) -> bool {
        return renderable->groupId() == PBR_POLY_HAVEN;
    });

    setViewportToResolution();
    clearColor(0.0f, 0.0f, 0.0f, 1.0f);

    _mainStage = addRenderStage(std::make_shared<lithium::RenderStage>(_frameBuffer, [this](){
        static const GLuint viewOffset{static_cast<GLuint>(sizeof(glm::mat4))};
        static const GLuint eyePosOffset{static_cast<GLuint>(sizeof(glm::mat4) * 2)};
        _cameraUBO->bufferSubData(viewOffset, _camera->view());
        _cameraUBO->bufferSubData(eyePosOffset, _camera->position());
        clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        disableDepthWriting();
        _skyboxGroup->render(_skyboxShader);
        enableDepthWriting();

        _pbrGroup->render(_pbrShader.get());
        _pbrBaseColorGroup->render(_pbrBaseColorShader.get());
        _pbrPolyHavenGroup->render(_pbrPolyHavenShader.get());
        _pbrDiffuseGroup->render(_pbrDiffuseShader.get());
    }));

    _finalStage = addRenderStage(std::make_shared<lithium::RenderStage>(nullptr, [this](){
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

Pipeline::~Pipeline()
{
    _msaaShader = nullptr;
    _screenMesh = nullptr;
}