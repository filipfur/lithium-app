#include "app.h"

#include "assetfactory.h"
#include "glplane.h"

App::App() : Application{"lithium-lab", glm::ivec2{1440, 800}, lithium::Application::Mode::MULTISAMPLED_4X, false}
{
    // Load all assets from the filesystem.
    AssetFactory::loadMeshes();
    AssetFactory::loadTextures();
    AssetFactory::loadObjects();
    AssetFactory::loadFonts();
    
    // Create the render pipeline
    _pipeline = std::make_shared<Pipeline>(defaultFrameBufferResolution());

    // Create and add a background plane to the render pipeline, and stage it for rendering.
    _background = std::make_shared<lithium::Object>(AssetFactory::getMeshes()->screen,
        std::vector<lithium::Object::TexturePointer>{});
    _background->setGroupId(Pipeline::BACKGROUND);
    _pipeline->attach(_background.get());
    _background->stage();

    auto basicTexture = lithium::ImageTexture::Basic();
    // Create and add a cube to the render pipeline, and stage it for rendering.
    auto monkey = std::make_shared<lithium::Object>(AssetFactory::getMeshes()->monkey,
        std::vector<lithium::Object::TexturePointer>{basicTexture});
    monkey->setPosition(glm::vec3{0.0f});
    monkey->setScale(1.0f);
    monkey->setColor(glm::vec3{0.5f, 0.0f, 0.5f});
    _pipeline->attach(monkey.get());
    _objects.push_back(monkey);
    monkey->stage();

    auto plane = std::make_shared<lithium::Object>(std::shared_ptr<lithium::Mesh>(lithium::Plane3D(glm::vec3(40.0f))),
        std::vector<lithium::Object::TexturePointer>{basicTexture});
    plane->setQuaternion(glm::angleAxis(glm::radians(-90.0f), glm::vec3{1.0f, 0.0f, 0.0f}));
    plane->setPosition(glm::vec3{0.0f, -1.0f, 0.0f});

    _objects.push_back(plane);
    _pipeline->attach(plane.get());
    plane->stage();

    // Key cache for rotating the camera left and right.
    _keyCache = std::make_shared<lithium::Input::KeyCache>(
        std::initializer_list<int>{GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_UP, GLFW_KEY_DOWN});
    input()->setKeyCache(_keyCache);

    // Escape key to close the application.
    input()->addPressedCallback(GLFW_KEY_ESCAPE, [this](int key, int mods) {
        this->close();
        return true;
    });

    setMaxFps(120.0f);

    // Set the camera oirigin position and target.
    _pipeline->camera()->setPosition(glm::vec3{3.0f, 3.0f, 3.0f});
    _pipeline->camera()->setTarget(glm::vec3{0.0f});

    printf("%s\n", glGetString(GL_VERSION));
}

App::~App() noexcept
{
    _pipeline = nullptr;
    _background = nullptr;
    _objects.clear();
}

void App::update(float dt)
{
    lithium::Updateable::update(dt);
    // Apply a rotation to the cube.
    for(auto o : _objects)
    {
        o->update(dt);
    }

    _pipeline->setTime(time());

    //auto o = _objects.front();
    //o->setRotation(o->rotation() + glm::vec3{0.0f, 4.0f * dt, 0.0f});

    // Rotate the camera around the origin on player input.
    if(_keyCache->isPressed(GLFW_KEY_LEFT))
    {
        _cameraAngle -= glm::pi<float>() * 0.5f * dt;
    }
    else if(_keyCache->isPressed(GLFW_KEY_RIGHT))
    {
        _cameraAngle += glm::pi<float>() * 0.5f * dt;
    }
    if(_keyCache->isPressed(GLFW_KEY_UP))
    {
        _camY += 5.0f * dt;
    }
    else if(_keyCache->isPressed(GLFW_KEY_DOWN))
    {
        _camY -= 5.0f * dt;
    }
    static const float cameraRadius = 6.0f;
    float camX = sin(_cameraAngle) * cameraRadius;
    float camZ = cos(_cameraAngle) * cameraRadius;
    _pipeline->camera()->setPosition(glm::vec3{camX, _camY, camZ});
    _pipeline->render();
}

void App::onWindowSizeChanged(int width, int height)
{
    _pipeline->setResolution(glm::ivec2{width, height});
}