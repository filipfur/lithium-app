#include "glapplication.h"
#include "pipeline.h"
#include "assetfactory.h"
#include "glplot.h"

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
        _object->setPosition(glm::vec3{0.0f});
        _object->setScale(1.0f);
        _renderPipeline = std::make_shared<Pipeline>(defaultFrameBufferResolution());
        _renderPipeline->attach(_object.get());
        _objects.push_back(_object);
        _object->stage();

        _plot = std::make_shared<Plot>(windowResolution(), 4096);
        _plot->setLineWidth(4.0f);
        const float widthIncrement = windowResolution().x / (2.0f * glm::pi<float>());
        const float heightIncrement = windowResolution().y / 2.0f;
        const glm::vec2 scale{widthIncrement, heightIncrement};
        const glm::vec2 center{windowResolution().x * 0.5f, windowResolution().y * 0.5f};
        for(float x = -glm::pi<float>(); x < glm::pi<float>(); x += 0.01f)
        {
            //float y = glm::sin(x * 4.0f);
            float y = glm::tanh(x);
            _plot->addPoint(center + glm::vec2{x, y} * scale);
        }

        input()->addPressedCallback(GLFW_KEY_ESCAPE, [this](int key, int mods) {
            this->close();
            return true;
        });

        printf("%s\n", glGetString(GL_VERSION));
    }

    virtual ~App() noexcept
    {
        _renderPipeline = nullptr;
    }

    virtual void update(float dt) override
    {
        for(auto o : _objects)
        {
            o->update(dt);
            o->setRotation(o->rotation() + glm::vec3{8.0f * dt});
        }
        _renderPipeline->camera()->update(dt);
        _renderPipeline->render();
        glClear(GL_DEPTH_BUFFER_BIT);
        glDisable(GL_CULL_FACE);
        _plot->render();
        glEnable(GL_CULL_FACE);
    }

    virtual void onFramebufferResized(int width, int height)
    {
        glViewport(0, 0, width, height);
    }

private:
    std::shared_ptr<Pipeline> _renderPipeline{nullptr};
    std::vector<std::shared_ptr<lithium::Object>> _objects;
    std::shared_ptr<lithium::Object> _object;
    std::shared_ptr<Plot> _plot;
};

int main(int argc, const char* argv[])
{
    std::unique_ptr<App> app = std::make_unique<App>();
    app->run();
    return 0;
}