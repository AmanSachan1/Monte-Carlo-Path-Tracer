#pragma once
#include <globals.h>

class Transform
{
public:
    Transform();
    Transform(const glm::vec3 &t, const glm::vec3 &r, const glm::vec3 &s);

    void SetMatrices();

    const glm::mat4 &T() const;
    const glm::mat3 T3() const;
    const glm::mat4 &invT() const;
    const glm::mat3 &invTransT() const;

    const glm::vec3 &position() const {return translation;}
    const glm::vec3 &getScale() const {return scale;}

private:
    glm::vec3 translation;
    glm::vec3 rotation;
    glm::vec3 scale;

    glm::mat4 worldTransform;
    glm::mat4 inverse_worldTransform;
    glm::mat3 inverse_transpose_worldTransform;
};
