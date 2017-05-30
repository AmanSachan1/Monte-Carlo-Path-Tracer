#include "diffusearealight.h"

Color3f DiffuseAreaLight::L(const Intersection &isect, const Vector3f &w) const
{
    if((glm::dot(isect.normalGeometric, w)>0.0f) || (twoSided))
    {
        return emittedLight;
    }
    else
    {
        return Color3f(0.f);
    }
}

Color3f DiffuseAreaLight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                     Vector3f *wi, Float *pdf) const
{
    Intersection intersection = shape->Sample(ref, xi, pdf);

    if(*pdf == 0.0f || ref.point == intersection.point)
    {
        return Color3f(0.0f);
    }
    *wi = glm::normalize(intersection.point - ref.point);

    return L(intersection, -*wi);
}

float DiffuseAreaLight::Pdf_Li(const Intersection &ref, const Vector3f &wi) const
{
    //we are calculating the solid angle subtended by the light source
    // 1/((cosTheta/r*r)*area) of the solid angle subtended by this is the pdf of the light source
    Intersection isect;
    Ray ray = ref.SpawnRay(wi);
    bool objectHit = shape->Intersect(ray, &isect);

    if(!objectHit)
    {
        return 0.0f;
    }

    float dist_sq = glm::length2(ref.point - isect.point);

    float _cosTheta = AbsDot(isect.normalGeometric, -wi);
    return dist_sq/(_cosTheta*shape->Area());
}
