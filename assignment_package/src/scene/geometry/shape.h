#pragma once

#include <globals.h>
#include <raytracing/intersection.h>
#include <openGL/drawable.h>
#include <raytracing/ray.h>
#include <scene/transform.h>
#include <scene/bounds.h>

class Intersection;//Forward declaration because Intersection and Geometry cross-include each other
class Material;

//Geometry is an abstract class since it contains a pure virtual function (i.e. a virtual function that is set to 0)
class Shape : public Drawable
{
public:
    static pcg32 colorRNG;

    Shape()
    {}

    virtual ~Shape(){}
    // Find the intersection of the ray with this Shape
    virtual bool Intersect(const Ray &ray, Intersection *isect) const = 0;
    virtual Point2f GetUVCoordinates(const Point3f &point) const = 0;
    // Given a t parameterization of the Ray, compute all requisite intersection information
    virtual void InitializeIntersection(Intersection* isect, float t, Point3f pLocal) const;

    // A helper function for computing the world-space normal, tangent, and bitangent at local-space point P
    virtual void ComputeTBN(const Point3f& P, Normal3f* nor, Vector3f* tan, Vector3f* bit) const = 0;

    // Compute and return the surface area of the shape in world space
    // i.e. you must account for the Transform applied to the Shape
    virtual float Area() const = 0;

    // Invokes the Sample below and converts the PDF from an area measure
    // to a solid angle measure
    virtual Intersection Sample(const Intersection &ref, const Point2f &xi,
                               float *pdf) const;

    // Sample a point on the surface of the shape and return the PDF with
    // respect to area on the surface.
    virtual Intersection Sample(const Point2f &xi, Float *pdf) const = 0;

    //Get a point on the surface of the light in accordance to some warping function
    virtual Point3f getPointOnSurface(const Point2f &xi) const = 0;

    float Pdf(const Intersection &ref, const Vector3f &wi) const;

    virtual Bounds3f WorldBound() const = 0;
};
