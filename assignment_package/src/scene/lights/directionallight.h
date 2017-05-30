#pragma once

#include "light.h"

class DirectionalLight: public Light
{
public:
    DirectionalLight( const Transform &t, const Color3f& Le,
                      const Vector3f &wLight )
        :Light(t), emittedLight(Le),
          wLight( glm::normalize( glm::vec3(transform.T() * glm::vec4(wLight,0.0f)) ) )
    {}

    virtual Color3f Sample_Li(const Intersection &ref, const Point2f &xi,
                      Vector3f *wi, Float *pdf) const;

    virtual Color3f L(const Intersection &isect, const Vector3f &w) const;
    virtual float Pdf_Li(const Intersection &ref, const Vector3f &wi) const;

    // Member variables
    const Color3f emittedLight;
    const Vector3f wLight;
    Point3f worldCenter;
    float worldRadius;
};
