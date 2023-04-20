#include "assetfactory.h"

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

static const std::vector<attr> screenMeshAttributes = { POSITION, NORMAL, UV };

static const std::vector<GLfloat> screenMeshVertices = {
    -1.0, -1.0, 0.0f, 	0.0f, 1.0f, 0.0f,	0.0f, 0.0f,
    -1.0,  1.0, 0.0f, 	0.0f, 1.0f, 0.0f,	0.0f, 1.0, 
    1.0,  1.0, 0.0f,	0.0f, 1.0f, 0.0f,	1.0, 1.0,  
    1.0, -1.0, 0.0f, 	0.0f, 1.0f, 0.0f,	1.0, 0.0f
};

static const std::vector<GLfloat> billboardMeshVertices = {
    -0.5f, -0.0f, +0.0f, 	+0.0f, +1.0f, +0.0f,	+0.0f, +0.0f,
    -0.5f, +1.0f, +0.0f, 	+0.0f, +1.0f, +0.0f,	+0.0f, +1.0f,
    +0.5f, +1.0f, +0.0f,	+0.0f, +1.0f, +0.0f,	+1.0f, +1.0f,
    +0.5f, -0.0f, +0.0f, 	+0.0f, +1.0f, +0.0f,	+1.0f, +0.0f,
};

static const std::vector<GLuint> screenMeshIndices = {
    0, 2, 1,
    0, 3, 2
};

void AssetFactory::loadMeshes()
{
    AssetFactory& instance = getInstance();
    instance._meshes.screen.reset(new lithium::Mesh(AssetFactory::objectAttributes, screenMeshVertices, screenMeshIndices));
    instance._meshes.cube = lithium::tinyobjloader_load("assets/block.obj", objectAttributes);
}

void AssetFactory::loadTextures()
{
    AssetFactory& instance = getInstance();
    instance._textures.logoDiffuse.reset((lithium::ImageTexture*)lithium::ImageTexture::load("assets/Kraxbox_logo_lithium_metal_2ff2069c-b84a-426c-bf92-e9831105a5df.png", GL_SRGB_ALPHA, GL_RGBA)->setFilter(GL_NEAREST));
    instance._textures.noiseTexture.reset(new lithium::Texture<unsigned char>(nullptr, 2048, 2048, GL_FLOAT, GL_RGBA32F, GL_RGBA, GL_TEXTURE_2D));
    instance._textures.noiseTexture->bind();
    instance._textures.noiseTexture->bindImageTexture(0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    instance._textures.noiseTexture->setFilter(GL_LINEAR);
    instance._textures.noiseTexture->unbind();
    instance._textures.computeTextureOut.reset(new lithium::Texture<unsigned char>(nullptr, 2048, 2048, GL_FLOAT, GL_RGBA32F, GL_RGBA, GL_TEXTURE_2D));
    instance._textures.computeTextureOut->bind();
    instance._textures.computeTextureOut->bindImageTexture(1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    instance._textures.computeTextureOut->setFilter(GL_LINEAR);
    instance._textures.computeTextureOut->unbind();
}

void AssetFactory::loadObjects()
{
    AssetFactory& instance = getInstance();
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
