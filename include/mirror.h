#pragma once

#include "globject.h"
#include "glshaderprogram.h"

class Mirror : public lithium::Object
{
public:
    Mirror(std::shared_ptr<lithium::Mesh> mesh, std::vector<lithium::Object::TexturePointer> textures) : lithium::Object{mesh, textures}
    {

    }

    Mirror(const Mirror& other) : lithium::Object{other}
    {

    }

    Mirror* clone() const
    {
        return new Mirror(*this);
    }

    virtual ~Mirror() noexcept
    {

    }

    void setReflectionNormal(const glm::vec3& reflectionNormal)
    {
        _reflectionNormal = reflectionNormal;
    }

    const glm::vec3& reflectionNormal() const
    {
        return _reflectionNormal;
    }
    
private:
    glm::vec3 _reflectionNormal{0.0f, 1.0f, 0.0f};

};