#pragma once
#include "integrator.h"

//Similar to the FullLighting Integrator except it has been implemented in a very naive manner.
//It is similar to the FullLighting Integrator except that it has no form of biassing, and thus takes an extremely
//large number of samples to converge to a nice image.

class NaiveIntegrator : public Integrator
{
public:
    NaiveIntegrator(Bounds2i bounds, Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit)
        : Integrator(bounds, s, sampler, recursionLimit)
    {}

    // Evaluate the energy transmitted along the ray back to its origin, e.g. the camera or an intersection in the scene
    virtual Color3f Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const;

    // Estimate lighting using MIS
    Color3f EstimateDirectLighting(const Ray &r, const Scene &scene, std::shared_ptr<Sampler> &sampler, Intersection &intersection) const;
};
