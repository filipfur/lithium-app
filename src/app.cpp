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

    // Create and add a cube to the render pipeline, and stage it for rendering.
    auto cube = std::make_shared<lithium::Object>(AssetFactory::getMeshes()->cube,
        std::vector<lithium::Object::TexturePointer>{AssetFactory::getTextures()->logoDiffuse});
    cube->setPosition(glm::vec3{0.0f, -9.0f, 0.0f});
    cube->setScale(8.0f);

    auto smallCube = std::make_shared<lithium::Object>(AssetFactory::getMeshes()->cube,
        std::vector<lithium::Object::TexturePointer>{AssetFactory::getTextures()->logoDiffuse});

    // Key cache for rotating the camera left and right.
    _keyCache = std::make_shared<lithium::Input::KeyCache>(
        std::initializer_list<int>{GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_UP, GLFW_KEY_DOWN,
            GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_LEFT_SHIFT});
    input()->setKeyCache(_keyCache);

    // Escape key to close the application.
    input()->addPressedCallback(GLFW_KEY_ESCAPE, [this](int key, int mods) {
        this->close();
        return true;
    });

    input()->addPressedCallback(GLFW_KEY_SPACE, [this](int key, int mods) {
        _playerController.jumping = true;
        return true;
    });

    input()->addReleasedCallback(GLFW_KEY_SPACE, [this](int key, int mods) {
        _playerController.jumping = false;
        return true;
    });

    // Set the camera oirigin position and target.
    _pipeline->camera()->setTarget(_cameraTarget);

    setMaxFps(120.0f);

    auto sceneLevel0 = std::make_shared<lithium::Scene>();
    sceneLevel0->addObject(cube);
    sceneLevel0->addObject(smallCube);
    sceneLevel0->attach(_pipeline);

    lithium::GameState gameStateLevel0;
    gameStateLevel0.addScene(sceneLevel0);
    _gameStates.push_back(std::move(gameStateLevel0));
    _gameStates.front().enter();

    Component::Time::attach(_playerEntity);
    Component::RigidBody::attach(_playerEntity);
    Component::Jump::attach(_playerEntity);
    _entities.insert(&_playerEntity);

    _entityObjects.emplace(&_playerEntity, smallCube);

    printf("%s\n", glGetString(GL_VERSION));
}

App::~App() noexcept
{
    _pipeline = nullptr;
    _background = nullptr;
    _gameStates.clear();
    _entityObjects.clear();
}

void App::update(float dt)
{
    lithium::Updateable::update(dt);
    // Apply a rotation to the cube.

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

    _playerController.forwardCtrl = PlayerController::ForwardControl::Idle;
    _playerController.sidewaysCtrl = PlayerController::SidewaysControl::Idle;
    if(_keyCache->isPressed(GLFW_KEY_W))
    {
        _playerController.forwardCtrl = PlayerController::ForwardControl::Forward;
    }
    else if(_keyCache->isPressed(GLFW_KEY_S))
    {
        _playerController.forwardCtrl = PlayerController::ForwardControl::Backward;
    }
    if(_keyCache->isPressed(GLFW_KEY_A))
    {
        _playerController.sidewaysCtrl = PlayerController::SidewaysControl::Left;
    }
    else if(_keyCache->isPressed(GLFW_KEY_D))
    {
        _playerController.sidewaysCtrl = PlayerController::SidewaysControl::Right;
    }

    glm::vec3 delta;
    switch(_playerController.forwardCtrl)
    {
    case PlayerController::ForwardControl::Forward:
        switch(_playerController.sidewaysCtrl)
        {
        case PlayerController::SidewaysControl::Left:
            delta = glm::normalize(glm::vec3{-1.0f, 0.0f, -1.0f});
            break;
        case PlayerController::SidewaysControl::Right:
            delta = glm::normalize(glm::vec3{+1.0f, 0.0f, -1.0f});
            break;
        case PlayerController::SidewaysControl::Idle:
            delta = glm::vec3{0.0f, 0.0f, -1.0f};
            break;
        }
        break;
    case PlayerController::ForwardControl::Backward:
        switch(_playerController.sidewaysCtrl)
        {
        case PlayerController::SidewaysControl::Left:
            delta = glm::normalize(glm::vec3{-1.0f, 0.0f, +1.0f});
            break;
        case PlayerController::SidewaysControl::Right:
            delta = glm::normalize(glm::vec3{+1.0f, 0.0f, +1.0f});
            break;
        case PlayerController::SidewaysControl::Idle:
            delta = glm::vec3{0.0f, 0.0f, +1.0f};
            break;
        }
        break;
    case PlayerController::ForwardControl::Idle:
        switch(_playerController.sidewaysCtrl)
        {
        case PlayerController::SidewaysControl::Left:
            delta = glm::vec3{-1.0f, 0.0f, 0.0f};
            break;
        case PlayerController::SidewaysControl::Right:
            delta = glm::vec3{+1.0f, 0.0f, 0.0f};
            break;
        case PlayerController::SidewaysControl::Idle:
            delta = glm::vec3{0.0f, 0.0f, 0.0f};
            break;
        }
        break;
    }


    RigidBody* playerRB = _playerEntity.get<Component::RigidBody>();

    static float movementSpeed{4.0f};
    playerRB->velocity.x = delta.x * movementSpeed;
    playerRB->velocity.z = delta.z * movementSpeed;

    static Time timeObj;
    timeObj.deltaTime = dt;
    timeObj.time = time();
    Component::Time::set(timeObj);

    static ecs::System<const Component::Time, Component::RigidBody> movementSystem;
    static ecs::System<const Component::Time, Component::RigidBody, Component::Jump> jumpSystem;

    jumpSystem.update(_entities, [this](ecs::Entity& entity, const Time& time, RigidBody& rigidBody, Jump& jump) {
        if(&entity != &_playerEntity)
        {
            return; // only handle player
        }
        switch(jump.state)
        {
            case Jump::State::Idle:
                jump.hasDouble = true;
                if(_playerController.jumping && rigidBody.onGround)
                {
                    jump.jumpDuration = 0.2f;
                    jump.state = Jump::State::Jumping;
                    rigidBody.onGround = false;
                    rigidBody.position.y += 0.01f;
                    _playerController.jumping = false;
                }
                break;
            case Jump::State::Jumping:
                jump.jumpDuration -= time.deltaTime;
                rigidBody.velocity.y = 8.0f;
                if(jump.jumpDuration <= 0.0f)
                {
                    jump.jumpDuration = 0.0f;
                    jump.state = Jump::State::Falling;
                }
                break;
            case Jump::State::Falling:
                if(jump.hasDouble && _playerController.jumping)
                {
                    jump.hasDouble = false;
                    jump.state = Jump::State::Jumping;
                    jump.jumpDuration = 0.2f;
                }
                if(rigidBody.onGround)
                {
                    jump.state = Jump::State::Idle;
                }
                else
                {
                    rigidBody.velocity.y = std::max(-10.0f, rigidBody.velocity.y - 24.0f * time.deltaTime);
                }
                break;
        }
    });

    movementSystem.update(_entities, [this](ecs::Entity& entity, const Time& time, RigidBody& rigidBody) {
        rigidBody.position += rigidBody.velocity * time.deltaTime;
        if(rigidBody.position.y < 0.0f)
        {
            rigidBody.position.y = 0.0f;
            rigidBody.velocity.y = 0.0f;
            rigidBody.onGround = true;
        }
    });

    for(auto entry : _entityObjects)
    {
        ecs::Entity* e = entry.first;
        auto& o = entry.second;
        o->setPosition(e->get<Component::RigidBody>()->position);
        o->update(dt);
    }

    static const float cameraRadius = 16.0f;

    _cameraTarget = _playerEntity.get<Component::RigidBody>()->position;

    glm::vec3 cameraPosition;
    cameraPosition.x = _cameraTarget.x + cameraRadius * cos(_cameraYaw) * cos(_cameraPitch);
    cameraPosition.y = _cameraTarget.y + cameraRadius * sin(_cameraPitch);
    cameraPosition.z = _cameraTarget.z + cameraRadius * sin(_cameraYaw) * cos(_cameraPitch);

    _pipeline->camera()->setPosition(cameraPosition);
    _pipeline->camera()->setTarget(_cameraTarget);
    _pipeline->render();
}

void App::onWindowSizeChanged(int width, int height)
{
    _pipeline->setResolution(glm::ivec2{width, height});
}