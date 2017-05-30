#pragma once
#include <globals.h>
#include <scene/materials/bsdf.h>
#include <raytracing/ray.h>
#include <scene/geometry/primitive.h>
#include <QList>
#include <scene/geometry/primitive.h>

class Material;
class BSDF;
class Primitive;

//The superclass that stores intersection information with the scene

//Has additional Functionality such as using the surface information to spawn a new ray using the appropriate offsets along the surface normal

class Intersection
{
public:
    Intersection();

    // Ask _objectHit_ to produce a BSDF
    // based on other data stored in this
    // Intersection, e.g. the surface normal
    // and UV coordinates
    bool ProduceBSDF();

    // Returns the light emitted from this Intersection
    // along _wo_, provided that this Intersection is
    // on the surface of an AreaLight. If not, then
    // zero light is returned.
    Color3f Le(const Vector3f& wo) const;

    // Instantiate a Ray that originates from this Intersection and
    // travels in direction d.
    Ray SpawnRay(const Vector3f &d) const;

    Point3f point;          // The place at which the intersection occurred
    Normal3f normalGeometric; // The surface normal at the point of intersection, NO alterations like normal mapping applied
    Point2f uv;             // The UV coordinates computed at the intersection
    float t;                  // The parameterization for the ray (in world space) that generated this intersection.
                              // t is equal to the distance from the point of intersection to the ray's origin if the ray's direction is normalized.
    Primitive const * objectHit;     // The object that the ray intersected, or nullptr if the ray hit nothing.
    std::shared_ptr<BSDF> bsdf;// The Bidirection Scattering Distribution Function found at the intersection.

    Vector3f tangent, bitangent; // World-space vectors that form an orthonormal basis with the surface normal.
};
