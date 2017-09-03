#include "pointlights.h"

//DO NOT PUT POINT LIGHTS CLOSE TO OTHER OBJECTS

Color3f PointLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                  Vector3f *wi, Float *pdf) const
{
    *wi = glm::normalize(pLight - ref.point);
    *pdf = 1.0f;

    return emittedLight/glm::distance2(pLight, ref.point);
}

Color3f PointLight::LightEmitted() const
{
    return emittedLight;
}

Ray PointLight::createPhotonRay( std::shared_ptr<Sampler> sampler ) const
{
    Vector3f origin = pLight;

    Point3f pObj = WarpFunctions::squareToSphereUniform(sampler->Get2D());
    Point3f p = Point3f(transform.T() * glm::vec4(pObj.x, pObj.y, pObj.z, 1.0f));
    p += glm::normalize( transform.invTransT() *pObj ) * RayEpsilon;
    Vector3f dir = glm::normalize(p);

    Ray ray = Ray(origin, dir);
    return ray;
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
