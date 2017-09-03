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

Color3f EnvironmentLight::LightEmitted() const
{
    return emittedLight;
}

Ray EnvironmentLight::createPhotonRay( std::shared_ptr<Sampler> sampler ) const
{
    Point3f pObj = WarpFunctions::squareToSphereUniform(sampler->Get2D());
    Point3f p = Point3f(transform.T() * glm::vec4(pObj.x, pObj.y, pObj.z, 1.0f));
    p += glm::normalize( transform.invTransT() *pObj ) * RayEpsilon;
    Vector3f origin = p;

    Vector3f dir = glm::normalize(p - Vector3f(0.0f,0.0f,0.0f));
    Ray ray = Ray(origin, dir);
    return ray;
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
