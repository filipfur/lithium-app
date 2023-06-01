#include "app.h"

#include "assetfactory.h"

App::App() : Application{"lithium-lab", glm::ivec2{1440, 800}, lithium::Application::Mode::MULTISAMPLED_4X, false}
{
    AssetFactory::loadMeshes();
    AssetFactory::loadTextures();
    AssetFactory::loadObjects();
    AssetFactory::loadFonts();
    
    _pipeline = std::make_shared<Pipeline>(defaultFrameBufferResolution());

    auto cube = std::make_shared<lithium::Object>(AssetFactory::getMeshes()->cube,
        std::vector<lithium::Object::TexturePointer>{AssetFactory::getTextures()->logoDiffuse});
    cube->setPosition(glm::vec3{0.0f, 0.0f, -1.5f});
    _pipeline->attach(cube.get());
    _objects.push_back(cube);
    cube->stage();

    auto cube2 = std::shared_ptr<lithium::Object>(cube->clone());
    cube2->setPosition(glm::vec3{0.0f, 0.0f, 1.5f});
    cube2->setShaderCallback([this](lithium::Renderable* r, lithium::ShaderProgram* shaderProgram) {
        shaderProgram->setTime(time());
    });
    cube2->setGroupId(1337);
    _pipeline->attach(cube2.get());
    _objects.push_back(cube2);
    cube2->stage();

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
    float camX = cos(_cameraAngle) * cameraRadius;
    static const float camY = cameraRadius * 0.5f;
    float camZ = sin(_cameraAngle) * cameraRadius;

    _pipeline->camera()->setPosition(glm::vec3{camX, camY, camZ});
    _pipeline->render();
}

void App::onFramebufferResized(int width, int height)
{
    glViewport(0, 0, width, height);
}