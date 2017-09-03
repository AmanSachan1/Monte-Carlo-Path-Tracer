#pragma once

#include "light.h"
#include <warpfunctions.h>

class Spotlight: public Light
{
public:
    Spotlight(const Transform &t, const Color3f& Le,
              float totalWidth, float falloffStart):
        Light(t), pLight(glm::vec3( transform.T()*glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) )),
        emittedLight(Le),
        cosTotalWidth(std::cos(glm::radians(totalWidth))),
        cosFallOffStart(std::cos(glm::radians(falloffStart)))
    {}

    virtual Color3f Sample_Li(const Intersection &ref, const Point2f &xi,
                      Vector3f *wi, Float *pdf) const;
    float Falloff(const Vector3f &w) const;

    virtual Color3f L(const Intersection &isect, const Vector3f &w) const;
    virtual float Pdf_Li(const Intersection &ref, const Vector3f &wi) const;

    Color3f LightEmitted() const;
    Ray createPhotonRay(std::shared_ptr<Sampler> sampler ) const;

    // Member variables
    const Color3f emittedLight;
    const Point3f pLight;
    float cosTotalWidth;
    float cosFallOffStart;
};
