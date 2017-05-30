#include "pointlights.h"

//DO NOT PUT POINT LIGHTS CLOSE TO OTHER OBJECTS

Color3f PointLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                  Vector3f *wi, Float *pdf) const
{
    *wi = glm::normalize(pLight - ref.point);
    *pdf = 1.0f;

    return emittedLight/glm::distance2(pLight, ref.point);
}

float PointLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    return 0.0f;
}

Color3f PointLight::L(const Intersection &isect, const Vector3f &w) const
{
    if(glm::dot(isect.normalGeometric, w)>0.99f && isect.t > glm::distance(isect.point, pLight))
    {
        return emittedLight;
    }
    else
    {
        return Color3f(0.0f);
    }
}
