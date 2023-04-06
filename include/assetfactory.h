#pragma once

#include "glfont.h"
#include "globject.h"
#include "glanimatedobject.h"
#include "gltinyobjloader.h"
#include "gltfloader.h"
#include "glskinnedobject.h"

class AssetFactory
{
public:
    struct Meshes
    {
        std::shared_ptr<lithium::Mesh> cube;
        std::shared_ptr<lithium::Mesh> screen;
        std::shared_ptr<lithium::Mesh> shack;
        std::shared_ptr<lithium::Mesh> floor;
        std::shared_ptr<lithium::Mesh> inside;
    } _meshes;

    struct Textures
    {
        std::shared_ptr<lithium::ImageTexture> logoDiffuse;
        std::shared_ptr<lithium::ImageTexture> woodDiffuse;
        std::shared_ptr<lithium::ImageTexture> insideDiffuse;
    } _textures;

    struct Objects
    {

    } _objects;

    struct Fonts
    {
        std::shared_ptr<lithium::Font> righteousFont;
    } _fonts;

    AssetFactory();
    virtual ~AssetFactory() noexcept;
    AssetFactory(AssetFactory const&) = delete;
    void operator=(AssetFactory const&)  = delete;

    static void loadMeshes();
    static void loadTextures();
    static void loadObjects();
    static void loadFonts();

    static AssetFactory& getInstance();

    static const std::vector<lithium::VertexArrayBuffer::AttributeType> objectAttributes;
    static const std::vector<lithium::VertexArrayBuffer::AttributeType> modelAttributes;

    static const AssetFactory::Meshes* getMeshes();
    static const AssetFactory::Textures* getTextures();
    static const AssetFactory::Objects* getObjects();
    static const AssetFactory::Fonts* getFonts();

    gltf::Loader _gltfLoader;
};