#include "app.h"
#include "assetfactory.h"
#include "glplane.h"

App::App() : Application{"lithium-lab", glm::ivec2{1440, 800}, lithium::Application::Mode::MULTISAMPLED_4X, false}
{
    AssetFactory::loadMeshes();
    AssetFactory::loadTextures();
    AssetFactory::loadObjects();
    AssetFactory::loadFonts();
    
    _pipeline = std::make_shared<Pipeline>(defaultFrameBufferResolution());

    auto cube = std::make_shared<lithium::Object>(AssetFactory::getMeshes()->cube,
        std::vector<lithium::Object::TexturePointer>{AssetFactory::getTextures()->logoDiffuse});
    cube->setPosition(glm::vec3{-2.5f, 0.0f, 0.0f});
    cube->setScale(0.5f);
    _pipeline->attach(cube.get());
    _objects.push_back(cube);
    cube->stage();

    auto cube2 = std::shared_ptr<lithium::Object>(cube->clone());
    cube2->setPosition(glm::vec3{2.5f, 0.0f, 0.0f});
    cube2->setShaderCallback([this](lithium::Renderable* r, lithium::ShaderProgram* shaderProgram) {
        shaderProgram->setTime(time());
    });
    cube2->setGroupId(1337);
    _pipeline->attach(cube2.get());
    _objects.push_back(cube2);
    cube2->stage();

    auto mesh = std::shared_ptr<lithium::Mesh>(
        new lithium::Mesh({lithium::VertexArrayBuffer::AttributeType::VEC3},
        {0.0f, 0.0f, 0.0f})
    );
    mesh->setDrawMode(GL_POINTS);

    input()->addPressedCallback(GLFW_KEY_A, [this](int key, int mods) {
        _rotation--;
        if(_rotation < 0)
            _rotation = 3;
        return true;
    });

    input()->addPressedCallback(GLFW_KEY_D, [this](int key, int mods) {
        _rotation++;
        if(_rotation > 3)
            _rotation = 0;
        return true;
    });

    auto plane = std::make_shared<lithium::Object>(mesh,
        std::vector<lithium::Object::TexturePointer>{AssetFactory::getTextures()->logoDiffuse});
    plane->setGroupId(2);
    _pipeline->attach(plane.get());
    _objects.push_back(plane);
    plane->setShaderCallback([this](lithium::Renderable* r, lithium::ShaderProgram* shaderProgram){
        shaderProgram->setTime(time());
        glm::vec3 right = glm::normalize(glm::vec3(
            cos(_rotation * glm::pi<float>() * 0.5f),
            0,
            sin(_rotation * glm::pi<float>() * 0.5f)));
        shaderProgram->setUniform("u_up", glm::vec3{0.0f, 1.0f, 0.0f});
        shaderProgram->setUniform("u_right", right);
    });
    plane->stage();

    _keyCache = std::make_shared<lithium::Input::KeyCache>(
        std::initializer_list<int>{GLFW_KEY_LEFT, GLFW_KEY_RIGHT});
    input()->setKeyCache(_keyCache);

    input()->addPressedCallback(GLFW_KEY_ESCAPE, [this](int key, int mods) {
        this->close();
        return true;
    });

    printf("%s\n", glGetString(GL_VERSION));

    float lineWidth[2];
    glGetFloatv(GL_LINE_WIDTH_RANGE, lineWidth);
    printf("supported line width: %.1f, %.1f\n", lineWidth[0], lineWidth[1]);
}

App::~App() noexcept
{
    _pipeline = nullptr;
    _objects.clear();
}

void App::update(float dt)
{
    /*for(auto o : _objects)
    {
        o->update(dt);
        o->setRotation(o->rotation() + glm::vec3{8.0f * dt});
    }*/
    lithium::Updateable::update(dt);

    if(_keyCache->isPressed(GLFW_KEY_LEFT))
    {
        _cameraAngle -= glm::pi<float>() * 0.5f * dt;
    }
    else if(_keyCache->isPressed(GLFW_KEY_RIGHT))
    {
        _cameraAngle += glm::pi<float>() * 0.5f * dt;
    }

    static const float cameraRadius = 6.0f;
    float camX = sin(_cameraAngle) * cameraRadius;
    static const float camY = cameraRadius * 0.5f;
    float camZ = cos(_cameraAngle) * cameraRadius;

    _pipeline->camera()->setPosition(glm::vec3{camX, camY, camZ});
    _pipeline->render();
}

void App::onFramebufferResized(int width, int height)
{
    glViewport(0, 0, width, height);
}