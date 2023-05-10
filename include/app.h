#pragma once

#include "glapplication.h"
#include "pipeline.h"
#include "assetfactory.h"
#include "glplot.h"

class App : public lithium::Application
{
public:
    App();

    virtual ~App() noexcept;

    virtual void update(float dt) override;

    virtual void onFramebufferResized(int width, int height);

    void fitNeuralNetwork();

private:
    std::shared_ptr<Pipeline> _pipeline{nullptr};
    std::vector<std::shared_ptr<lithium::Object>> _objects;
    std::shared_ptr<lithium::Object> _object;
    std::shared_ptr<Plot> _sigmoidPlot;
    std::shared_ptr<Plot> _sigmoidDPlot;
    std::shared_ptr<Plot> _reluPlot;
    std::shared_ptr<Plot> _reluDPlot;
};