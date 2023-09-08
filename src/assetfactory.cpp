#include "assetfactory.h"

#include "glplane.h"

AssetFactory::AssetFactory()
{

}

AssetFactory::~AssetFactory() noexcept
{

}

using attr = lithium::VertexArrayBuffer::AttributeType;
static constexpr attr POSITION{attr::VEC3};
static constexpr attr NORMAL{attr::VEC3};
static constexpr attr UV{attr::VEC2};
static constexpr attr BONE_IDS{attr::VEC4};
static constexpr attr BONE_WEIGHTS{attr::VEC4};
static constexpr attr COLOR{attr::VEC3};

const std::vector<attr> AssetFactory::objectAttributes{POSITION, NORMAL, UV};
const std::vector<attr> AssetFactory::modelAttributes{POSITION, NORMAL, UV, BONE_IDS, BONE_WEIGHTS};

void AssetFactory::loadMeshes()
{
    AssetFactory& instance = getInstance();
    instance._meshes.screen.reset(lithium::Plane2D());
    instance._meshes.cube = lithium::tinyobjloader_load("assets/block.obj", objectAttributes);
    instance._meshes.sphere = lithium::tinyobjloader_load("assets/sphere.obj", objectAttributes);
}

void loadTextureMaps(std::vector<std::shared_ptr<lithium::Texture<unsigned char>>>& textureMaps, const std::string& path)
{
    textureMaps.push_back(std::shared_ptr<lithium::ImageTexture>(
        lithium::ImageTexture::load(std::filesystem::exists(path + "_basecolor_boosted.png")
            ? (path + "_basecolor_boosted.png")
            : (path + "_basecolor.png"), GL_RGB, GL_RGB)));
    
    if(std::filesystem::exists(path + "_normal.png"))
    {
        textureMaps.push_back(std::shared_ptr<lithium::ImageTexture>(
            lithium::ImageTexture::load(path + "_normal.png", GL_RGB, GL_RGB)));
    }
    if(std::filesystem::exists(path + "_metallic.png"))
    {
        textureMaps.push_back(std::shared_ptr<lithium::ImageTexture>(
            lithium::ImageTexture::load(path + "_metallic.png", GL_RED, GL_RED)));
    }
    if(std::filesystem::exists(path + "_roughness.png"))
    {
        textureMaps.push_back(std::shared_ptr<lithium::ImageTexture>(
            lithium::ImageTexture::load(path + "_roughness.png", GL_RED, GL_RED)));
    }

    if(std::filesystem::exists(path + "_ao.png"))
    {
        textureMaps.push_back(std::shared_ptr<lithium::ImageTexture>(
            lithium::ImageTexture::load(path + "_ao.png", GL_RGB, GL_RGB)));
    }
}

void AssetFactory::loadTextures()
{
    AssetFactory& instance = getInstance();
    instance._textures.logoDiffuse.reset((lithium::ImageTexture*)lithium::ImageTexture::load("assets/Kraxbox_logo_lithium_metal_2ff2069c-b84a-426c-bf92-e9831105a5df.png", GL_SRGB_ALPHA, GL_RGBA)->setFilter(GL_NEAREST));
    loadTextureMaps(instance._textures.rustedIron, "assets/rustediron1-alt2-bl/rustediron2");
    loadTextureMaps(instance._textures.scuffedGold, "assets/gold-scuffed-bl/gold-scuffed");

    instance._textures.barberChair.push_back(std::shared_ptr<lithium::ImageTexture>(
        lithium::ImageTexture::load("assets/BarberShopChair_01_4k/textures/BarberShopChair_01_diff_4k.jpg", GL_SRGB, GL_RGB, 4, false)));
    instance._textures.barberChair.push_back(std::shared_ptr<lithium::ImageTexture>(
        lithium::ImageTexture::load("assets/BarberShopChair_01_4k/textures/BarberShopChair_01_nor_gl_4k.jpg", GL_RGB, GL_RGB, 4, false)));
    instance._textures.barberChair.push_back(std::shared_ptr<lithium::ImageTexture>(
        lithium::ImageTexture::load("assets/BarberShopChair_01_4k/textures/BarberShopChair_01_arm_4k.jpg", GL_RGB, GL_RGB,  4, false)));
}

void AssetFactory::loadObjects()
{
    AssetFactory& instance = getInstance();
    instance._gltfLoader.loadObjects(instance._objects.barberChair, "assets/BarberShopChair_01_4k/BarberShopChair_01_4k.gltf");
}

void AssetFactory::loadFonts()
{
    AssetFactory& instance = getInstance();
}

const AssetFactory::Meshes* AssetFactory::getMeshes()
{
    return &AssetFactory::getInstance()._meshes;
}

const AssetFactory::Textures* AssetFactory::getTextures()
{
    return &AssetFactory::getInstance()._textures;
}

const AssetFactory::Objects* AssetFactory::getObjects()
{
    return &AssetFactory::getInstance()._objects;
}

const AssetFactory::Fonts* AssetFactory::getFonts()
{
    return &AssetFactory::getInstance()._fonts;
}

AssetFactory& AssetFactory::getInstance()
{
    static AssetFactory instance;
    return instance;
}
