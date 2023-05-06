#include "glapplication.h"
#include "pipeline.h"
#include "assetfactory.h"
#include "mirror.h"

using TextureArray = std::vector<lithium::Object::TexturePointer>;

class App : public lithium::Application
{
public:
    App() : Application{"lithium-lab", glm::ivec2{1440, 800}, lithium::Application::Mode::DEFAULT, false}
    {
        AssetFactory::loadMeshes();
        AssetFactory::loadTextures();
        AssetFactory::loadObjects();
        AssetFactory::loadFonts();

        auto object = std::make_shared<lithium::Object>(AssetFactory::getMeshes()->chess,
            TextureArray{AssetFactory::getTextures()->marbleLoresDiffuse});
        object->setPosition(glm::vec3{0.0f, 0.0f, 0.0f});

        auto cube = std::make_shared<lithium::Object>(AssetFactory::getMeshes()->cube,
            TextureArray{AssetFactory::getTextures()->logoDiffuse});
        cube->setScale(0.2f);
        cube->setPosition(0.5f, 0.8f, 0.2f);
        cube->setRotation(glm::vec3{45.0f});

        auto plane = std::make_shared<Mirror>(AssetFactory::getMeshes()->plane,
            TextureArray{AssetFactory::getTextures()->marbleDiffuse});
        plane->setScale(1.5f);

        auto wall = std::make_shared<Mirror>(AssetFactory::getMeshes()->cube,
            TextureArray{AssetFactory::getTextures()->marbleDiffuse});
        wall->setPosition(1.7f, 1.5f, 0.0f);
        wall->setScale(glm::vec3{0.2f, 1.5f, 1.5f});
        wall->setReflectionNormal(glm::vec3{-1.0f, 0.0f, 0.0f});
        
        _renderPipeline = new Pipeline(defaultFrameBufferResolution());
        _renderPipeline->attach(object.get());
        _renderPipeline->attach(cube.get());
        _renderPipeline->attach(plane.get());
        _renderPipeline->attach(wall.get());
        object->stage();
        cube->stage();
        plane->stage();
        wall->stage();
        _objects.push_back(object);
        _objects.push_back(cube);
        _objects.push_back(plane);
        _objects.push_back(wall);

        printf("%s\n", glGetString(GL_VERSION));
    }

    virtual ~App() noexcept
    {
    }

    virtual void update(float dt) override
    {
        float t = time() * 0.5f;
        float camRadius = 3.0f;
        float camX = camRadius * cos(t);
        float camZ = camRadius * sin(t);
        _renderPipeline->camera()->setPosition(glm::vec3{camX, 1.0f, camZ});
        _renderPipeline->camera()->update(dt);
        _renderPipeline->render();
    }

    virtual void onFramebufferResized(int width, int height)
    {
        glViewport(0, 0, width, height);
    }

private:
    Pipeline* _renderPipeline{nullptr};
    std::vector<std::shared_ptr<lithium::Object>> _objects;
};

int main(int argc, const char* argv[])
{
    std::unique_ptr<App> app = std::make_unique<App>();
    app->run();
    return 0;
}