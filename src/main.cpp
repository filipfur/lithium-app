#include "glapplication.h"
#include "pipeline.h"
#include "assetfactory.h"

class App : public lithium::Application
{
public:
    App() : Application{"lithium-lab", glm::ivec2{1440, 800}, lithium::Application::Mode::MULTISAMPLED_4X, false}
    {
        AssetFactory::loadMeshes();
        AssetFactory::loadTextures();
        AssetFactory::loadObjects();
        AssetFactory::loadFonts();
        _object = std::shared_ptr<lithium::Object>(new lithium::Object(AssetFactory::getMeshes()->cube,
                                                                       {AssetFactory::getTextures()->logoDiffuse}));
        _object->setPosition(glm::vec3{0.0f, 1.0f, 0.0f});
        _object->setScale(glm::vec3{1.0f, 2.0f, 0.5f});
        _renderPipeline = new Pipeline(defaultFrameBufferResolution());

        _floor = std::shared_ptr<lithium::Object>(new lithium::Object(AssetFactory::getMeshes()->floor,
                                                                     {AssetFactory::getTextures()->woodDiffuse}));
        _floor->setScale(3.0f);
        _renderPipeline->addRenderable(_floor.get());

        _inside = std::shared_ptr<lithium::Object>(new lithium::Object(AssetFactory::getMeshes()->inside,
                                                                      {AssetFactory::getTextures()->insideDiffuse}));
        _inside->setPosition(glm::vec3{0.0f, 1.0f, 0.0f});
                                                                    
        _renderPipeline->addSecondary(_inside.get());

        _house = std::shared_ptr<lithium::Object>(new lithium::Object(AssetFactory::getMeshes()->house,
                                                                     {AssetFactory::getTextures()->houseDiffuse}));

        _house->setRotation(glm::vec3{0.0f, -90.0f, 0.0f});

        _house->setPosition(glm::vec3{0.0f, 0.0f, 1.0f});

        _renderPipeline->addRenderable(_house.get());

        _stencil = std::shared_ptr<lithium::Object>(new lithium::Object(AssetFactory::getMeshes()->stencil,
                                                                       {}));

        _stencil->setPosition(glm::vec3{0.0f, 0.0f, 1.0f});
        _stencil->setRotation(glm::vec3{0.0f, -90.0f, 0.0f});

        _renderPipeline->addFacade(_stencil.get());

        _door = std::shared_ptr<lithium::Object>(new lithium::Object(AssetFactory::getMeshes()->door,
                                                                    {AssetFactory::getTextures()->houseDiffuse}));

        _door->setPosition(glm::vec3{0.0f, 0.0f, 1.0f});
        _door->setRotation(glm::vec3{0.0f, -90.0f, 0.0f});

        _renderPipeline->addRenderable(_door.get());

        input()->addPressedCallback(GLFW_KEY_ESCAPE, [this](int key, int mods) {
            exit(0);
            return true;
        });

        input()->addPressedCallback(GLFW_KEY_TAB, [this](int key, int mods) {
            _renderPipeline->setStencilDebug(1.0f);
            return true;
        });
        input()->addReleasedCallback(GLFW_KEY_TAB, [this](int key, int mods) {
            _renderPipeline->setStencilDebug(0.0f);
            return true;
        });

    }

    virtual ~App() noexcept
    {
    }

    virtual void update(float dt) override
    {
        /*for (auto o : _objects)
        {
            o->update(dt);
            o->setRotation(o->rotation() + glm::vec3{8.0f * dt});
        }*/

        // rotate camera around target
        _renderPipeline->camera()->setTarget(glm::vec3{0.0f, 0.0f, 0.0f});

        float t = sinf(time()) * 0.5f;
        float rotX = sinf(t);
        float rotZ = cosf(t);
        _renderPipeline->camera()->setPosition(glm::vec3{6.0f * rotX, 2.0f, 6.0f * rotZ});


        _renderPipeline->camera()->update(dt);
        _renderPipeline->render();
    }

    virtual void onFramebufferResized(int width, int height)
    {
        glViewport(0, 0, width, height);
    }

private:
    Pipeline *_renderPipeline{nullptr};
    std::vector<std::shared_ptr<lithium::Object>> _objects;
    std::shared_ptr<lithium::Object> _object;
    std::shared_ptr<lithium::Object> _shack;
    std::shared_ptr<lithium::Object> _floor;
    std::shared_ptr<lithium::Object> _inside;
    std::shared_ptr<lithium::Object> _house;
    std::shared_ptr<lithium::Object> _stencil;
    std::shared_ptr<lithium::Object> _door;
};

int main(int argc, const char *argv[])
{
    std::unique_ptr<App> app = std::make_unique<App>();
    app->run();
    return 0;
}