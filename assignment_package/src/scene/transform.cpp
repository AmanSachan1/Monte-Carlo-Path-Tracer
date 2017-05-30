#include <scene/transform.h>

Transform::Transform():
    Transform(glm::vec3(0), glm::vec3(0), glm::vec3(1))
{}

Transform::Transform(const glm::vec3 &t, const glm::vec3 &r, const glm::vec3 &s):
    translation(t),
    rotation(r),
    scale(s)
{
    SetMatrices();
}

void Transform::SetMatrices()
{
    worldTransform = glm::translate(glm::mat4(1.0f), translation)
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1,0,0))
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0,1,0))
            * glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0,0,1))
            * glm::scale(glm::mat4(1.0f), scale);
    inverse_worldTransform = glm::inverse(worldTransform);
    inverse_transpose_worldTransform = glm::mat3(glm::inverse(glm::transpose(worldTransform)));
}

const glm::mat4& Transform::T() const
{
    return worldTransform;
}
const glm::mat3 Transform::T3() const
{
    return glm::mat3(worldTransform);
}
const glm::mat4& Transform::invT() const
{
    return inverse_worldTransform;
}
const glm::mat3& Transform::invTransT() const
{
    return inverse_transpose_worldTransform;
}
