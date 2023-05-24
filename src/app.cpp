#include "app.h"

#include "assetfactory.h"
#include "shape.h"
#include "utility.h"
#include "glgeometry.h"
#include "glaabb.h"

App::App() : Application{"lithium-lab", glm::ivec2{1440, 800}, lithium::Application::Mode::MULTISAMPLED_4X, false}
{
    AssetFactory::loadMeshes();
    AssetFactory::loadTextures();
    AssetFactory::loadObjects();
    AssetFactory::loadFonts();
    
    _pipeline = std::make_shared<Pipeline>(defaultFrameBufferResolution());

    for(int i{0}; i < 20; ++i)
    {
        auto cube = new lithium::Entity(AssetFactory::getMeshes()->cube,
            std::vector<lithium::Object::TexturePointer>{AssetFactory::getTextures()->logoDiffuse});
        cube->setPosition(glm::vec3{0.0f, 2.0f, 0.0f}
            + glm::vec3{utility::randn(-1.0f, 1.0f), utility::randn(-1.0f, 1.0f), utility::randn(-1.0f, 1.0f)});
        _pipeline->attach(cube);
        _entities.emplace(cube);
        // rotate randomly along all axis
        float pi2 = glm::pi<float>() * 2.0f;
        /*cube->setRotation(glm::angleAxis(utility::randn(0.0f, pi2), glm::vec3{1.0f, 0.0f, 0.0f})
            * glm::angleAxis(utility::randn(0.0f, pi2), glm::vec3{0.0f, 1.0f, 0.0f})
            * glm::angleAxis(utility::randn(0.0f, pi2), glm::vec3{0.0f, 0.0f, 1.0f}));*/
        cube->setScale(glm::vec3{utility::randn(0.1f, 0.2f)});
        cube->stage();
        component::Time::attach(*cube);
        component::Collider::attach(*cube);
        //component::Force::attach(*cube);
        component::RigidBody::attach(*cube);
        component::Gravity::attach(*cube);
        glm::vec3 scaleHalf = cube->scale();
        cube->get<component::Collider>()->geometry = new lithium::AABB(cube->position(), -scaleHalf, scaleHalf);
    }

    auto cube = new lithium::Entity(AssetFactory::getMeshes()->cube,
            std::vector<lithium::Object::TexturePointer>{AssetFactory::getTextures()->logoDiffuse});
    cube->setPosition(glm::vec3{0.0f, -1.0f, 0.0f});
    _pipeline->attach(cube);
    _entities.emplace(cube);
    cube->stage();
    component::Time::attach(*cube);
    component::Collider::attach(*cube);
    //component::Force::attach(*cube);
    component::RigidBody::attach(*cube);
    glm::vec3 scaleHalf = cube->scale();
    cube->get<component::Collider>()->geometry = new lithium::AABB(cube->position(), -scaleHalf, scaleHalf);
    //component::Gravity::attach(*cube);
    /*auto planeMesh = std::shared_ptr<lithium::Mesh>(shape::Plane());
    unsigned char blueColor[] = {0x00, 0xA0, 0xFF};
    auto blueTexture = std::make_shared<lithium::Texture<unsigned char>>(
        blueColor, 1, 1, GL_UNSIGNED_BYTE, GL_RGB, GL_RGB
    );
    auto plane = new lithium::Entity(planeMesh,
        std::vector<lithium::Object::TexturePointer>{blueTexture});
    plane->setScale(glm::vec3{2.0f});
    _pipeline->attach(plane);
    _entities.emplace(plane);
    plane->setRotation(glm::angleAxis(glm::pi<float>() * 0.5f, glm::vec3{1.0f, 0.0f, 0.0f}));
    plane->stage();*/

    _keyCache = std::make_shared<lithium::Input::KeyCache>(
        std::initializer_list<int>{GLFW_KEY_LEFT, GLFW_KEY_RIGHT});
    input()->setKeyCache(_keyCache);

    input()->addPressedCallback(GLFW_KEY_ESCAPE, [this](int key, int mods) {
        this->close();
        return true;
    });

    printf("%s\n", glGetString(GL_VERSION));
}

App::~App() noexcept
{
    _pipeline = nullptr;
    for(auto entity : _entities)
    {
        delete entity;
    }
    _entities.clear();
}

void App::update(float dt)
{
    /*for(auto o : _entities)
    {
        o->update(dt);
    }*/

    auto& time = component::Time::get();
    time.seconds += dt;
    time.delta = dt;
    component::Time::set(time);

    _gravitySystem.update(_entities, [](ecs::Entity& entity, const Time& time, const Gravity& gravity, RigidBody& rigidBody) {
        if(gravity.active)
        {
            rigidBody.force = glm::vec3{0.0f, -9.81f * rigidBody.mass, 0.0f};
        }
    });

    _forceSystem.update(_entities, [](ecs::Entity& entity, const Time& time, Force& force, RigidBody& rigidBody) {
        for(auto& f : force.forces)
        {
            rigidBody.force += f.second;
            f.first -= time.delta;
        }
        force.forces.erase(std::remove_if(force.forces.begin(), force.forces.end(),
            [](const std::pair<float, glm::vec3>& f) {
                return f.first <= 0.0f;
            }), force.forces.end());
    });

    _physicsSystem.update(_entities, [](ecs::Entity& entity, const Time& time, RigidBody& rigidBody, glm::vec3& translation) {
        glm::vec3 acceleration = rigidBody.force / rigidBody.mass;
        rigidBody.velocity += acceleration * time.delta;
        rigidBody.velocity *= 1.0f - rigidBody.drag * time.delta;
        translation += rigidBody.velocity * time.delta + 0.5f * acceleration * time.delta * time.delta;
        rigidBody.force = glm::vec3{0.0f};
    });

    _transformationSystem.update(_entities, [](ecs::Entity& entity,
        const glm::vec3& position,
        const glm::quat& rotation,
        const glm::vec3& scale,
        glm::mat4& modelMatrix){
        modelMatrix = glm::scale(glm::translate(glm::mat4{1.0f}, position) * glm::toMat4(rotation), scale);
        /*std::cout << "updating model matrix." << std::endl;
        std::cout << " pos:" << utility::GLMtoString(position, 3, 2) << std::endl;
        std::cout << " rotation:" << utility::GLMtoString(rotation, 4, 2) << std::endl;
        std::cout << " scale:" << utility::GLMtoString(scale, 3, 2) << std::endl;*/
    });

    _collisionSystem.tick(_entities);
        
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
    _pipeline->camera()->update(dt);
    _pipeline->render();
}

void App::onFramebufferResized(int width, int height)
{
    glViewport(0, 0, width, height);
}