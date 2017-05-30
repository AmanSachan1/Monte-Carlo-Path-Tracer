#include "environmentlighting.h"

Color3f EnvironmentLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                  Vector3f *wi, Float *pdf) const
{
    glm::vec3 rayDir = glm::normalize(-(*wi));

    float u = 0.5 + (std::atan2(rayDir.z/Pi, rayDir.x/Pi))*Inv2Pi;
    float v = 0.5 + rayDir.y * 0.5;

    Point2f uv = Point2f(u,v);

    Color3f lightColor = Material::GetImageColor(uv, this->environmentMap.get());
    return lightColor*intensity;
}

float EnvironmentLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    return 1.0f;
}

Color3f EnvironmentLight::L(const Intersection &isect, const Vector3f &w) const
{
    //only called if the intersection actually hit an object
    glm::vec3 rayDir = glm::normalize(w);

    float u = 0.5 + (std::atan2(rayDir.z/Pi, rayDir.x/Pi))*Inv2Pi;
    float v = 0.5 + rayDir.y * 0.5;

    Point2f uv = Point2f(u,v);

    Color3f lightColor = Material::GetImageColor(uv, this->environmentMap.get());
    return lightColor*intensity;
}
