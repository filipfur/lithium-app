#include "app.h"

#include "glplane.h"
#include "assetfactory.h"
#include "utility.h"

App::App() : Application{"lithium-lab", glm::ivec2{1440, 800}, lithium::Application::Mode::MULTISAMPLED_4X, false}
{
    AssetFactory::loadMeshes();
    AssetFactory::loadTextures();
    AssetFactory::loadObjects();
    AssetFactory::loadFonts();
    auto cube = std::make_shared<lithium::Object>(AssetFactory::getMeshes()->cube,
        std::vector<lithium::Object::TexturePointer>{AssetFactory::getTextures()->logoDiffuse});
    cube->setPosition(glm::vec3{0.0f});
    cube->setScale(1.0f);
    _pipeline = std::make_shared<Pipeline>(defaultFrameBufferResolution());
    _pipeline->attach(cube.get());
    _objects.push_back(cube);
    cube->stage();

    _keyCache = std::make_shared<lithium::Input::KeyCache>(
        std::initializer_list<int>{GLFW_KEY_LEFT, GLFW_KEY_RIGHT});
    input()->setKeyCache(_keyCache);

    input()->addPressedCallback(GLFW_KEY_ESCAPE, [this](int key, int mods) {
        this->close();
        return true;
    });

    auto res = windowResolution();

    glm::vec2 canvasDim{2048.0f};
    //glm::vec2 canvasDim{res};

    _canvas = std::make_shared<lithium::Canvas>(res, defaultFrameBufferResolution());
    _canvas->loadUI("assets/ui.json");

    auto canvasFrame = _canvas->frameById("canvas");
    canvasFrame->setMesh(std::shared_ptr<lithium::Mesh>(lithium::Plane2D(glm::vec2{0.5f}, glm::vec2{64.0f})));
    canvasFrame->setTextures(std::vector<lithium::Object::TexturePointer>{AssetFactory::getTextures()->checkboard});
    auto classFrame = _canvas->frameById("canvas.0");
    classFrame->setColor(glm::vec3{0.3f, 0.3f, 0.3f});
    auto titleFrame = _canvas->frameById("canvas.0.0");
    titleFrame->setColor(glm::vec3{0.2f, 0.2f, 0.2f});
    auto myText = titleFrame->createTextRenderer()->createText(AssetFactory::getFonts()->righteousFont, "Frame", 1.0f);
    myText->setPosition(glm::vec3{-myText->width() * 0.5f, -myText->height() * 0.5f, 0.0f});
    myText->setScale(1.0f);
    auto bodyFrame = _canvas->frameById("canvas.0.1");
    bodyFrame->setColor(glm::vec3{0.1f, 0.1f, 0.1f});
    _canvas->frameById("canvas.1")->setColor(glm::vec3{0.0f, 0.0f, 1.0f});

    static lithium::FrameLayout staticLayout{
        "",
        lithium::FrameLayout::Mode::Absolute,
        lithium::FrameLayout::Orientation::Vertical,
        lithium::FrameLayout::Alignment::Center,
        glm::vec4{0.0f},
        glm::vec4{0.0f},
        glm::vec2{128.0f},
        glm::vec2{-256.0f}
    };

    lithium::FrameLayout* layout = classFrame->layout()->clone()->setPosition(glm::vec2{0.0f, -64.0f});
    _canvas->refreshUI(); // TODO: Fix this refreshlayout stuff. Its bloating the code.
    auto classFrame2 = _canvas->addFrame(layout);
    lithium::FrameLayout* layout2 = canvasFrame->layout()->appendChild(staticLayout);
    _canvas->refreshUI(); // TODO: Fix this refreshlayout stuff. Its bloating the code.
    _canvas->addFrame(layout2);

    classFrame2->setColor(classFrame->color());
    classFrame2->refresh();
    _canvas->frameById("canvas.2.1")->setColor(bodyFrame->color());
    auto titleFrame2 = _canvas->frameById("canvas.2.0");
    titleFrame2->setColor(titleFrame->color());
    auto myText2 = titleFrame2->createTextRenderer()->createText(AssetFactory::getFonts()->righteousFont, "FrameRenderer", 1.0f);
    myText2->setPosition(glm::vec3{-myText2->width() * 0.5f, -myText2->height() * 0.5f, 0.0f});

    input()->setDragCallback([this](int button, int modifiers, const glm::vec2& start, const glm::vec2& current, const glm::vec2& delta, lithium::Input::DragState dragState) {
        if(button == GLFW_MOUSE_BUTTON_LEFT)
        {
            _canvas->move(glm::vec2(-delta.x, delta.y));
        }
    });

    printf("%s\n", glGetString(GL_VERSION));
}

App::~App() noexcept
{
    _pipeline = nullptr;
    _objects.clear();
}

void App::update(float dt)
{
    for(auto o : _objects)
    {
        o->update(dt);
        o->setRotation(o->rotation() + glm::vec3{8.0f * dt});
    }

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
    //_pipeline->render();

    _canvas->update(dt);

    /*
    Wiggle canvas!
    static float r{0.0f};
    static int currS{0};
    int iTime = static_cast<int>(time());
    if(iTime != currS)
    {
        currS = iTime;
        r = utility::randn(-glm::pi<float>(), glm::pi<float>());
    }
    glm::vec2 v = glm::vec2{cos(r), sin(r)};
    _canvas->move(v * 64.0f * dt);*/

    _canvas->render();
}

void App::onWindowSizeChanged(int width, int height)
{
    _pipeline->setResolution(glm::ivec2{width, height});
}