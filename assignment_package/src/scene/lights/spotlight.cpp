#include "spotlight.h"

Color3f Spotlight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                     Vector3f *wi, Float *pdf) const
{
    *wi = glm::normalize(pLight - ref.point);
    *pdf = 1.0f;

    return emittedLight*Falloff(-*wi)/glm::distance2(pLight, ref.point);
}

Color3f Spotlight::LightEmitted() const
{
    return emittedLight;
}

Ray Spotlight::createPhotonRay( std::shared_ptr<Sampler> sampler ) const
{
    Vector3f origin = pLight;

    float thetaMin = std::acos(cosTotalWidth);
    Point3f pObj = WarpFunctions::squareToSphereCapUniform(sampler->Get2D(), thetaMin);
    Point3f p = Point3f(transform.T() * glm::vec4(pObj.x, pObj.y, pObj.z, 1.0f));
    p += glm::normalize( transform.invTransT() *pObj ) * RayEpsilon;
    Vector3f dir = Vector3f(0.0f);

    Ray ray = Ray(origin, dir);
    return ray;
}

float Spotlight::Falloff(const Vector3f &w) const
{
    Vector3f wl = glm::normalize( glm::vec3(transform.invT() * glm::vec4(w, 0.0f)) );
    float cosTheta = wl.z;
    if(cosTheta < cosTotalWidth)
    {
        return 0.0f;
    }
    if(cosTheta > cosFallOffStart)
    {
        return 1.0f;
    }

    float delta = (cosTheta - cosTotalWidth)/
                  (cosFallOffStart - cosTotalWidth);

    return (delta*delta)*(delta*delta);
}

float Spotlight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    return 0.0f;
}

Color3f Spotlight::L(const Intersection &isect, const Vector3f &w) const
{
    return Color3f(0.f);
}
