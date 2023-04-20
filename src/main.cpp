#include "glapplication.h"
#include "pipeline.h"
#include "assetfactory.h"

class App : public lithium::Application
{
public:
    App() : Application{"lithium-lab", glm::ivec2{1024}, lithium::Application::Mode::MULTISAMPLED_4X, false}
    {
        AssetFactory::loadMeshes();
        AssetFactory::loadTextures();
        AssetFactory::loadObjects();
        AssetFactory::loadFonts();
        _object = std::shared_ptr<lithium::Object>(new lithium::Object(AssetFactory::getMeshes()->cube,
            {AssetFactory::getTextures()->logoDiffuse}));
        _object->setPosition(glm::vec3{0.0f});
        _object->setScale(1.0f);
        _renderPipeline = new Pipeline(defaultFrameBufferResolution());
        //_renderPipeline->addRenderable(_object.get());
        _objects.push_back(_object);
        printf("%s\n", glGetString(GL_VERSION));
        int work_grp_cnt[3];

        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

        printf("max global (total) work group counts x:%i y:%i z:%i\n",
        work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

        int work_grp_size[3];

        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

        printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n",
        work_grp_size[0], work_grp_size[1], work_grp_size[2]);

        int work_grp_inv;
        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
        printf("max local work group invocations %i\n", work_grp_inv);

        _simplexShader.reset(new lithium::ShaderProgram("shaders/simplex.glsl"));
        auto tex = AssetFactory::getTextures()->noiseTexture;
        _simplexShader->dispatchCompute(tex->width(), tex->height(), 1);
        // make sure writing to image has finished before read
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


        _rayShader.reset(new lithium::ShaderProgram("shaders/rays.glsl"));
        tex->bind(GL_TEXTURE0);
        tex = AssetFactory::getTextures()->computeTextureOut;
        _rayShader->dispatchCompute(tex->width(), tex->height(), 1);
        // make sure writing to image has finished before read
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    virtual ~App() noexcept
    {
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
    }

    virtual void onFramebufferResized(int width, int height)
    {
        glViewport(0, 0, width, height);
    }

private:
    Pipeline* _renderPipeline{nullptr};
    std::vector<std::shared_ptr<lithium::Object>> _objects;
    std::shared_ptr<lithium::Object> _object;
    std::shared_ptr<lithium::ShaderProgram> _simplexShader;
    std::shared_ptr<lithium::ShaderProgram> _rayShader;
};

int main(int argc, const char* argv[])
{
    std::unique_ptr<App> app = std::make_unique<App>();
    app->run();
    return 0;
}