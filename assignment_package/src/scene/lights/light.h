#pragma once
#include <globals.h>
#include <scene/transform.h>
#include <raytracing/intersection.h>

class Intersection;

class Light
{
  public:
    virtual ~Light(){}
    Light(Transform t)
        : transform(t), name(), flag_infiniteLight(false)
    {}

    // Returns the light emitted along a ray that does
    // not hit anything within the scene bounds.
    // Necessary if we want to support things like environment
    // maps, or other sources of light with infinite area.
    // The default implementation for general lights returns
    // no energy at all.
    virtual Color3f Le(const Ray &r) const;

    virtual Color3f L(const Intersection &isect, const Vector3f &w) const = 0;

    virtual Color3f Sample_Li(const Intersection &ref, const Point2f &xi,
                                                Vector3f *wi, Float *pdf) const = 0;
    virtual float Pdf_Li(const Intersection &ref, const Vector3f &wi) const = 0;

    virtual Color3f LightEmitted() const = 0;

    virtual Ray createPhotonRay( std::shared_ptr<Sampler> sampler ) const = 0;

    QString name; // For debugging
    bool flag_infiniteLight;
    const Transform transform;
};

class AreaLight : public Light
{
public:
    AreaLight(const Transform &t) : Light(t){}
    // Returns the light emitted from a point on the light's surface _isect_
    // along the direction _w_, which is leaving the surface.
    virtual Color3f L(const Intersection &isect, const Vector3f &w) const = 0;
    virtual float Pdf_Li(const Intersection &ref, const Vector3f &wi) const = 0;
    virtual Color3f LightEmitted() const = 0;
};
