#include "app.h"

#include "assetfactory.h"

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

    // Create and add a cube to the render pipeline, and stage it for rendering.
    auto cube = std::make_shared<lithium::Object>(AssetFactory::getMeshes()->cube,
        std::vector<lithium::Object::TexturePointer>{AssetFactory::getTextures()->logoDiffuse});
    cube->setPosition(glm::vec3{0.0f});
    cube->setScale(1.0f);
    _pipeline->attach(cube.get());
    _objects.push_back(cube);
    cube->stage();

    // Key cache for rotating the camera left and right.
    _keyCache = std::make_shared<lithium::Input::KeyCache>(
        std::initializer_list<int>{GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_UP, GLFW_KEY_DOWN});
    input()->setKeyCache(_keyCache);

    // Escape key to close the application.
    input()->addPressedCallback(GLFW_KEY_ESCAPE, [this](int key, int mods) {
        this->close();
        return true;
    });

    // Set the camera oirigin position and target.
    _pipeline->camera()->setTarget(_cameraTarget);

    setMaxFps(120.0f);

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
        /*o->setQuaternion(o->quaternion() * glm::angleAxis(0.5f * dt, glm::vec3(1,0,0))
            * glm::angleAxis(0.5f * dt, glm::vec3(0,1,0))
            * glm::angleAxis(0.5f * dt, glm::vec3(0,0,1)));*/
    }

    // Rotate the camera around the origin on player input.
    if(_keyCache->isPressed(GLFW_KEY_LEFT))
    {
        _cameraYaw += glm::pi<float>() * 0.5f * dt;
    }
    else if(_keyCache->isPressed(GLFW_KEY_RIGHT))
    {
        _cameraYaw -= glm::pi<float>() * 0.5f * dt;
    }

    if(_keyCache->isPressed(GLFW_KEY_UP))
    {
        _cameraPitch += glm::pi<float>() * 0.5f * dt;
    }
    else if(_keyCache->isPressed(GLFW_KEY_DOWN))
    {
        _cameraPitch -= glm::pi<float>() * 0.5f * dt;
    }

    static const float cameraRadius = 8.0f;

    glm::vec3 cameraPosition;
    cameraPosition.x = _cameraTarget.x + cameraRadius * cos(_cameraYaw) * cos(_cameraPitch);
    cameraPosition.y = _cameraTarget.y + cameraRadius * sin(_cameraPitch);
    cameraPosition.z = _cameraTarget.z + cameraRadius * sin(_cameraYaw) * cos(_cameraPitch);

    _pipeline->camera()->setPosition(cameraPosition);
    _pipeline->render();
}

void App::onWindowSizeChanged(int width, int height)
{
    _pipeline->setResolution(glm::ivec2{width, height});
}