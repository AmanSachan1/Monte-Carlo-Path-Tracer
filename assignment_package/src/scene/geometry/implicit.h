#pragma once
#include "shape.h"

//The implicit shape is assumed to be centered about <0,0,0>.
//These attributes can be altered by applying a transformation matrix to the shape.

class Implicit : public Shape
{
public:
    virtual bool Intersect(const Ray &ray, Intersection *isect) const;
    virtual Point2f GetUVCoordinates(const Point3f &point) const;
    virtual void ComputeTBN(const Point3f& P, Normal3f* nor, Vector3f* tan, Vector3f* bit) const;

    // Since ellipsoids should technically be a different
    // class of Shape than Sphere, you may assume that a Sphere
    // has been scaled uniformly when computing its surface area.
    virtual float Area() const;
    virtual Point3f getPointOnSurface(const Point2f &xi) const;

    // Sample a point on the surface of the shape and return the PDF with
    // respect to area on the surface.
    virtual Intersection Sample(const Point2f &xi, Float *pdf) const;

    Bounds3f WorldBound() const;

    void create();
};
