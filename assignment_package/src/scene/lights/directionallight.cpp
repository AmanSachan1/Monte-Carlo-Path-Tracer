#include "directionallight.h"

Color3f DirectionalLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                     Vector3f *wi, Float *pdf) const
{
    *wi = wLight;
    *pdf = 1.0f;
    Point3f pOutside = ref.point + wLight*2.0f*worldRadius;
    return emittedLight;
}

Color3f DirectionalLight::LightEmitted() const
{
    return emittedLight;
}

float DirectionalLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    return 0.0f;
}

Color3f DirectionalLight::L(const Intersection &isect, const Vector3f &w) const
{
    return Color3f(0.f);
}

Ray DirectionalLight::createPhotonRay( std::shared_ptr<Sampler> sampler ) const
{
    glm::vec4 pos = transform.T()*glm::vec4(0.0,0.0,0.0,1.0);
    Vector3f origin = Vector3f(pos.x, pos.y, pos.z);
    Vector3f dir = wLight;
    Ray ray = Ray(origin, dir);
    return ray;
}
