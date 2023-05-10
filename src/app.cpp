#include "app.h"

#include "dlmatrix.h"
#include "dlneuralnetwork.h"
#include "dlfunctions.h"

using mat = lithium::Matrix;

App::App() : Application{"lithium-lab", glm::ivec2{800, 800}, lithium::Application::Mode::MULTISAMPLED_4X, false}
{
    AssetFactory::loadMeshes();
    AssetFactory::loadTextures();
    AssetFactory::loadObjects();
    AssetFactory::loadFonts();
    _object = std::shared_ptr<lithium::Object>(new lithium::Object(AssetFactory::getMeshes()->cube,
        {AssetFactory::getTextures()->logoDiffuse}));
    _object->setPosition(glm::vec3{0.0f});
    _object->setScale(1.0f);
    _pipeline = std::make_shared<Pipeline>(defaultFrameBufferResolution());
    _pipeline->attach(_object.get());
    _objects.push_back(_object);
    _object->stage();

    _sigmoidPlot = std::make_shared<Plot>(windowResolution(), 4096);
    _sigmoidPlot->setLineWidth(4.0f);
    _sigmoidPlot->setColor(glm::vec3{1.0f, 1.0f, 0.0f});

    _sigmoidDPlot = std::make_shared<Plot>(windowResolution(), 4096);
    _sigmoidDPlot->setLineWidth(4.0f);
    _sigmoidDPlot->setColor(glm::vec3{1.0f, 0.0f, 1.0f});

    _reluPlot = std::make_shared<Plot>(windowResolution(), 4096);
    _reluPlot->setLineWidth(4.0f);
    _reluPlot->setColor(glm::vec3{0.0f, 1.0f, 1.0f});

    _reluDPlot = std::make_shared<Plot>(windowResolution(), 4096);
    _reluDPlot->setLineWidth(4.0f);
    _reluDPlot->setColor(glm::vec3{1.0f, 0.0f, 0.0f});

    const float widthIncrement = windowResolution().x / (2.0f * glm::pi<float>());
    const float heightIncrement = windowResolution().y / 2.0f;
    glm::vec2 scale{widthIncrement, heightIncrement};
    scale *= 0.7f;
    const glm::vec2 center{windowResolution().x * 0.5f, windowResolution().y * 0.5f};
    mat test(1, 1);
    for(float x = -glm::pi<float>(); x < glm::pi<float>(); x += 0.01f)
    {
        //float y = glm::sin(x * 4.0f);
        //float y = glm::tanh(x);
        test[0][0] = x;
        float y = lithium::sigmoid(test)[0][0];
        _sigmoidPlot->addPoint(center + glm::vec2{x, y} * scale);
        y = lithium::sigmoid_d(test)[0][0];
        _sigmoidDPlot->addPoint(center + glm::vec2{x, y} * scale);
        y = lithium::relu(test)[0][0];
        _reluPlot->addPoint(center + glm::vec2{x, y} * scale);
        y = lithium::relu_d(test)[0][0];
        _reluDPlot->addPoint(center + glm::vec2{x, y} * scale);
    }

    input()->addPressedCallback(GLFW_KEY_ESCAPE, [this](int key, int mods) {
        this->close();
        return true;
    });

    printf("%s\n", glGetString(GL_VERSION));

    fitNeuralNetwork();
}

App::~App() noexcept
{
    _pipeline = nullptr;
}

void App::fitNeuralNetwork()
{
    auto& assetFactory = AssetFactory::getInstance();
    //const size_t numElements = assetFactory._mnistImageWidth * assetFactory._mnistImageHeight;
    //mat input(assetFactory._mnistImageCount, numElements);
    //mat truth(assetFactory._mnistImageCount, 10, 0.0f);
    static constexpr size_t numSamples{60000};
    static constexpr size_t numElements{784};
    static mat input(numSamples, numElements);
    static mat truth(numSamples, 10);

    static mat hidden0 = mat::uniform(numElements, 128, 0.01f, 0.99f);
    static mat hidden1 = mat::uniform(hidden0.columns(), 64, 0.01f, 0.99f);

    for(size_t i{0}; i < assetFactory._mnistImageCount; ++i)
    {
        for(size_t j{0}; j < numElements; ++j)
        {
            input[i][j] = static_cast<float>(assetFactory._mnistImages[i * numElements + j]) / 255.0f;
        }
        truth[i][assetFactory._mnistLabels[i]] = 1.0f;
    }

    lithium::Layer inputLayer {
        mat{},
        std::make_pair(lithium::passthru, lithium::passthru),
        input
    };
    
    lithium::Layer denseLayer0 {
        hidden0,
        std::make_pair(lithium::relu, lithium::relu_d)
    };

    lithium::Layer denseLayer1 {
        hidden1,
        std::make_pair(lithium::relu, lithium::relu_d)
    };
    lithium::Layer outputLayer1 {
        mat::uniform(hidden1.columns(), 10, 0.01f, 0.99f),
        std::make_pair(lithium::softmax, lithium::softmax_d)
    };

    static lithium::NeuralNetwork net{
        inputLayer,
        denseLayer0,
        denseLayer1,
        outputLayer1
    };

    for(int epoch{0}; epoch < 30; ++epoch)
    {
        net.forward();
        mat error = truth - net.outputLayer()->activations;
        if(epoch % 5 == 0)
        {
            std::cout << "error.sum()" << error.sum();
        }
        mat dZ = error * 0.01f;
        net.backward(dZ);
    }

    /*std::cout << std::fixed << std::setprecision(1) << input << std::endl
        << truth << std::endl;*/

}

void App::update(float dt)
{
    for(auto o : _objects)
    {
        o->update(dt);
        o->setRotation(o->rotation() + glm::vec3{8.0f * dt});
    }
    _pipeline->camera()->update(dt);
    _pipeline->render();
    glClear(GL_DEPTH_BUFFER_BIT);
    //glDisable(GL_CULL_FACE);
    _sigmoidPlot->render();
    _sigmoidDPlot->render();
    _reluPlot->render();
    _reluDPlot->render();
    //glEnable(GL_CULL_FACE);
}

void App::onFramebufferResized(int width, int height)
{
    glViewport(0, 0, width, height);
}