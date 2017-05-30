#pragma once

#include "light.h"

class PointLight: public Light
{
public:
    PointLight(const Transform &t, const Color3f& Le)
        :Light(t), emittedLight(Le),
         pLight(glm::vec3( t.T()*glm::vec4(0.0f,0.0f,0.0f,1.0f) ))
     {}

     virtual Color3f Sample_Li(const Intersection &ref, const Point2f &xi,
                       Vector3f *wi, Float *pdf) const;

     virtual Color3f L(const Intersection &isect, const Vector3f &w) const;
     virtual float Pdf_Li(const Intersection &ref, const Vector3f &wi) const;

     // Member variables
     const Color3f emittedLight;
     const Point3f pLight;
};
