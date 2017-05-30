#pragma once

#include "light.h"

class DiffuseAreaLight : public AreaLight
{
public:
    DiffuseAreaLight(const Transform &t, const Color3f& Le,
                     const std::shared_ptr<Shape>& shape,
                     bool twoSided = false)
        : AreaLight(t), emittedLight(Le), shape(shape),
          area(shape->Area()), twoSided(twoSided)
    {}

    // Returns the energy emitted along _w_ from a point on our surface _isect_
    // If _twoSided_ is false, then we must check if _w_ is in the same direction
    // as the surface normal of _isect_. If we are looking at the back side of the
    // light, then this function returns zero color.
    //virtual Color3f L(const Intersection &isect, const Vector3f &w) const;
    Color3f L(const Intersection &isect, const Vector3f &w) const;

    // Given a point of intersection on some surface in the scene and a pair
    // of uniform random variables, generate a sample point on the surface
    // of our shape and evaluate the light emitted along the ray from
    // our Shape's surface to the reference point.
    Color3f Sample_Li(const Intersection &ref, const Point2f &xi,
                                         Vector3f *wi, Float *pdf) const;


    virtual float Pdf_Li(const Intersection &ref, const Vector3f &wi) const;

    // Member variables
    const Color3f emittedLight;
    std::shared_ptr<Shape> shape;
    float area;    // The surface area of our _shape_
    bool twoSided; // If true, then this emits light on both sides of its surface normal
};
