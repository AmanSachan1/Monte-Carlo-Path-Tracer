#include "spotlight.h"

Color3f Spotlight::Sample_Li(const Intersection &ref, const Point2f &xi,
                                     Vector3f *wi, Float *pdf) const
{
    *wi = glm::normalize(pLight - ref.point);
    *pdf = 1.0f;

    return emittedLight*Falloff(-*wi)/glm::distance2(pLight, ref.point);
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
