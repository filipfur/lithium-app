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

uint32_t swap_endianness(uint32_t value) {
    return ((value & 0x000000FF) << 24) |
           ((value & 0x0000FF00) << 8) |
           ((value & 0x00FF0000) >> 8) |
           ((value & 0xFF000000) >> 24);
}

void AssetFactory::loadTextures()
{
    AssetFactory& instance = getInstance();
    instance._textures.logoDiffuse.reset((lithium::ImageTexture*)lithium::ImageTexture::load("assets/Kraxbox_logo_lithium_metal_2ff2069c-b84a-426c-bf92-e9831105a5df.png", GL_SRGB_ALPHA, GL_RGBA)->setFilter(GL_NEAREST));

    

    std::string images_filepath = "assets/archive/train-images-idx3-ubyte/train-images-idx3-ubyte";
    std::ifstream file(images_filepath, std::ios::binary);

    if (!file)
    {
        std::cerr << "Error opening file: " << images_filepath << std::endl;
        exit(1);
    }

    uint32_t magic, size, rows, cols;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    magic = swap_endianness(magic); // Convert to host byte order

    if (magic != 2051) {
        throw std::runtime_error("Magic number mismatch, expected 2051, got " + std::to_string(magic));
    }

    file.read(reinterpret_cast<char*>(&size), sizeof(size));
    size = swap_endianness(size); // Convert to host byte order
    file.read(reinterpret_cast<char*>(&rows), sizeof(rows));
    rows = swap_endianness(rows); // Convert to host byte order
    file.read(reinterpret_cast<char*>(&cols), sizeof(cols));
    cols = swap_endianness(cols); // Convert to host byte order
    //std::vector<uint8_t> image_data(std::istreambuf_iterator<char>(file), {});
    instance._mnistImages = std::vector<unsigned char>(std::istreambuf_iterator<char>(file), {});
    file.close();

    instance._mnistImageCount = static_cast<size_t>(size);
    instance._mnistImageWidth = static_cast<size_t>(rows);
    instance._mnistImageHeight = static_cast<size_t>(cols);

    std::string labels_filepath = "assets/archive/train-labels-idx1-ubyte/train-labels-idx1-ubyte";
    file.open(labels_filepath, std::ios::binary);

    if(!file)
    {
        std::cerr << "Error opening file: " << labels_filepath << std::endl;
        exit(1);
    }

    //uint8_t magic8, size8;
    file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    magic = swap_endianness(magic); // Convert to host byte order
    file.read(reinterpret_cast<char*>(&size), sizeof(size));
    size = swap_endianness(size); // Convert to host byte order
    if (magic != 2049) {
        throw std::runtime_error("Magic number mismatch, expected 2049, got " + std::to_string(magic));
    }
    instance._mnistLabels = std::vector<unsigned char>(std::istreambuf_iterator<char>(file), {});
    file.close();

    instance._textures.mnistSamplesDiffuse.reset((new lithium::Texture<unsigned char>(instance._mnistImages.data() + (28*28) * 31457, 28, 28, GL_UNSIGNED_BYTE, GL_RED, GL_RED))->setFilter(GL_NEAREST)->setWrap(GL_CLAMP_TO_EDGE)->setUnpackAlignment(1));
    std::cout << "mnist sample: " << (int)instance._mnistLabels[31457] << std::endl;
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
