#include "app.h"

#include "assetfactory.h"
#include "glcube.h"

App::App() : Application{"lithium-lab", glm::ivec2{1440, 800}, lithium::Application::Mode::MULTISAMPLED_4X, false}
{
    // Load all assets from the filesystem.
    AssetFactory::loadMeshes();
    AssetFactory::loadTextures();
    AssetFactory::loadObjects();
    AssetFactory::loadFonts();
    
    // Create the render pipeline
    _pipeline = std::make_shared<Pipeline>(defaultFrameBufferResolution());

    // Create and add a cube to the render pipeline, and stage it for rendering.
    auto cube = std::make_shared<lithium::Object>(AssetFactory::getMeshes()->cube,
        std::vector<lithium::Object::TexturePointer>{AssetFactory::getTextures()->logoDiffuse});
    cube->setPosition(glm::vec3{0.0f});
    cube->setScale(1.0f);
    _pipeline->attach(cube.get());
    _objects.push_back(cube);
    cube->stage();


    static constexpr size_t numSpheres = 15;

    static glm::vec3 spherePositions[numSpheres] = {
        glm::vec3{-2.0f, 0.5f, 0.0f},
        glm::vec3{-1.0f, 0.5f, 0.0f},
        glm::vec3{0.0f, 0.5f, 0.0f},
        glm::vec3{1.0f, 0.5f, 0.0f},
        glm::vec3{2.0f, 0.5f, 0.0f},
        glm::vec3{-2.0f, -0.5f, 0.0f},
        glm::vec3{-1.0f, -0.5f, 0.0f},
        glm::vec3{0.0f, -0.5f, 0.0f},
        glm::vec3{1.0f, -0.5f, 0.0f},
        glm::vec3{2.0f, -0.5f, 0.0f},
        glm::vec3{-2.0f, -1.5f, 0.0f},
        glm::vec3{-1.0f, -1.5f, 0.0f},
        glm::vec3{0.0f, -1.5f, 0.0f},
        glm::vec3{1.0f, -1.5f, 0.0f},
        glm::vec3{2.0f, -1.5f, 0.0f}
    };

    static float sphereMetallics[numSpheres] = {
        0.01f,
        0.25f,
        0.5f,
        0.75f,
        0.99f,
        0.01f,
        0.25f,
        0.5f,
        0.75f,
        0.99f,
        0.01f,
        0.25f,
        0.5f,
        0.75f,
        0.99f
    };

    static float sphereRoughnesses[numSpheres] = {
        0.01f,
        0.01f,
        0.01f,
        0.01f,
        0.01f,
        0.5f,
        0.5f,
        0.5f,
        0.5f,
        0.5f,
        0.99f,
        0.99f,
        0.99f,
        0.99f,
        0.99f,
    };

    static glm::vec3 sphereColors[numSpheres] = {
        glm::vec3{1.0f, 0.0f, 0.0f},
        glm::vec3{0.0f, 1.0f, 0.0f},
        glm::vec3{1.0f, 1.0f, 1.0f},
        glm::vec3{0.0f, 0.0f, 1.0f},
        glm::vec3{1.0f, 1.0f, 0.0f},
    };

    for(int i{0}; i < numSpheres; ++i)
    {
        auto sphere = std::make_shared<lithium::Object>(
            std::shared_ptr<lithium::Mesh>(AssetFactory::getMeshes()->sphere->clone()),
            std::vector<lithium::Object::TexturePointer>{});
        sphere->setPosition(spherePositions[i] + glm::vec3{0.0f, 0.0f, -2.0f});
        sphere->setScale(glm::vec3{0.4f});
        sphere->setGroupId(Pipeline::PBR);
        sphere->mesh()->setMaterial(std::make_shared<lithium::Material>(
            glm::vec4{sphereColors[i % 5], 1.0f},
            sphereMetallics[i],
            sphereRoughnesses[i]));
        _objects.push_back(sphere);
        _pipeline->attach(sphere.get());
        sphere->stage();
    }

    auto sphere = std::make_shared<lithium::Object>(
            std::shared_ptr<lithium::Mesh>(AssetFactory::getMeshes()->sphere->clone()),
            AssetFactory::getTextures()->rustedIron);
    sphere->setPosition(glm::vec3{0.0f, 1.5f, -2.0f});
    sphere->setScale(glm::vec3{0.4f});
    sphere->setGroupId(Pipeline::PBR);
    _objects.push_back(sphere);
    _pipeline->attach(sphere.get());
    sphere->stage();

    auto goldSphere = std::make_shared<lithium::Object>(
            std::shared_ptr<lithium::Mesh>(AssetFactory::getMeshes()->sphere->clone()),
            AssetFactory::getTextures()->scuffedGold);
    goldSphere->setPosition(glm::vec3{1.0f, 1.5f, -2.0f});
    goldSphere->setScale(glm::vec3{0.4f});
    goldSphere->setGroupId(Pipeline::PBR);
    _objects.push_back(goldSphere);
    _pipeline->attach(goldSphere.get());
    goldSphere->stage();

    _cubemapHDR = lithium::CubemapHDR::load("assets/wrestling_gym_8k.hdr");
    _cubemapHDR->brdfLUT()->bind(GL_TEXTURE7);
    _cubemapHDR->irradianceMap()->bind(GL_TEXTURE8);
    _cubemapHDR->prefilterMap()->bind(GL_TEXTURE9);

    _skybox = std::shared_ptr<lithium::Object>(new lithium::Object(std::shared_ptr<lithium::Mesh>(lithium::InvCube()), {
        /*AssetFactory::getTextures()->skybox*/}));
    _skybox->setGroupId(Pipeline::SKYBOX);
    _skybox->setShaderCallback([this](lithium::Renderable* r, lithium::ShaderProgram* shaderProgram) {
        shaderProgram->setUniform("u_lod", _lod);
    });
    _pipeline->attach(_skybox.get());
    _skybox->stage();

    for(auto o : AssetFactory::getObjects()->barberChair)
    {
        o->setTextures(AssetFactory::getTextures()->barberChair);
        o->setGroupId(Pipeline::PBR_POLY_HAVEN);
        _pipeline->attach(o.get());
        o->stage();
        o->setScale(2.0f);
    }

    // Key cache for rotating the camera left and right.
    _keyCache = std::make_shared<lithium::Input::KeyCache>(
        std::initializer_list<int>{GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_UP, GLFW_KEY_DOWN});
    input()->setKeyCache(_keyCache);

    // Escape key to close the application.
    input()->addPressedCallback(GLFW_KEY_ESCAPE, [this](int key, int mods) {
        this->close();
        return true;
    });

    input()->addPressedCallback(GLFW_KEY_KP_ADD, [this](int key, int mods) {
        _lod += 0.25f;
        return true;
    });

    input()->addPressedCallback(GLFW_KEY_KP_SUBTRACT, [this](int key, int mods) {
        _lod -= 0.25f;
        _lod = std::max(0.0f, _lod);
        return true;
    });

    // Set the camera oirigin position and target.
    _pipeline->camera()->setPosition(glm::vec3{3.0f, 3.0f, 3.0f});
    _pipeline->camera()->setTarget(glm::vec3{0.0f});

    printf("%s\n", glGetString(GL_VERSION));
}

App::~App() noexcept
{
    _pipeline = nullptr;
    _objects.clear();
}

void App::update(float dt)
{
    lithium::Updateable::update(dt);

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
    float camX = sin(_cameraAngle) * _cameraRadius;
    float camZ = cos(_cameraAngle) * _cameraRadius;
    _pipeline->camera()->setPosition(_pipeline->camera()->target() + glm::vec3{camX, _camY, camZ});
    _cubemapHDR->bind(GL_TEXTURE0);
    _pipeline->setTime(time());
    _pipeline->render();
}

void App::onWindowSizeChanged(int width, int height)
{
    _pipeline->setResolution(glm::ivec2{width, height});
}