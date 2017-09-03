#pragma once

#include "light.h"
#include <src/scene/materials/material.h>
#include <warpfunctions.h>

class EnvironmentLight: public Light
{
public:
    EnvironmentLight(const Transform &t, const Color3f& Le, std::shared_ptr<QImage> &texture, const float intensity)
        : Light(t), emittedLight(Le), environmentMap(texture), intensity(intensity)
    {
        flag_infiniteLight = true;
    }

    virtual Color3f Sample_Li(const Intersection &ref, const Point2f &xi,
                   Vector3f *wi, Float *pdf) const;

    virtual Color3f L(const Intersection &isect, const Vector3f &w) const;
    virtual float Pdf_Li(const Intersection &ref, const Vector3f &wi) const;

    Color3f LightEmitted() const;
    Ray createPhotonRay(std::shared_ptr<Sampler> sampler ) const;

    // Member variables
    float intensity;
    const Color3f emittedLight;
    std::shared_ptr<QImage> environmentMap;
};
