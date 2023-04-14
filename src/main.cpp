#include "glapplication.h"
#include "pipeline.h"
#include "assetfactory.h"
#include "mirror.h"

class App : public lithium::Application
{
public:
    App() : Application{"lithium-lab", glm::ivec2{1440, 800}, lithium::Application::Mode::DEFAULT, false}
    {
        AssetFactory::loadMeshes();
        AssetFactory::loadTextures();
        AssetFactory::loadObjects();
        AssetFactory::loadFonts();
        //_object = std::shared_ptr<lithium::Object>(new lithium::Object(AssetFactory::getMeshes()->cube, {AssetFactory::getTextures()->logoDiffuse}));
        _object = std::shared_ptr<lithium::Object>(new lithium::Object(AssetFactory::getMeshes()->chess, {AssetFactory::getTextures()->marbleLoresDiffuse}));
        _object->setPosition(glm::vec3{0.0f, 0.0f, 0.0f});
        //_object->setScale(glm::vec3{0.5f});

        _plane = std::shared_ptr<Mirror>(new Mirror(AssetFactory::getMeshes()->plane,
            {AssetFactory::getTextures()->marbleDiffuse}));
        _plane->setScale(1.5f);
        
        _renderPipeline = new Pipeline(defaultFrameBufferResolution());
        _renderPipeline->addRenderable(_object.get());
        _renderPipeline->addRenderable(_plane.get());
        _objects.push_back(_object);
    }

    virtual ~App() noexcept
    {
    }

    virtual void update(float dt) override
    {
        /*for(auto o : _objects)
        {
            o->update(dt);
            o->setRotation(o->rotation() + glm::vec3{8.0f * dt});
        }*/
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
    std::shared_ptr<lithium::Object> _object;
    std::shared_ptr<Mirror> _plane;
};

int main(int argc, const char* argv[])
{
    std::unique_ptr<App> app = std::make_unique<App>();
    app->run();
    return 0;
}