#pragma once
#include "integrator.h"
#include "directlightingintegrator.h"
#include <scene/lights/light.h>

//Similar to the FullLighting Integrator except that it has can handle participating media in the scene.

//The integrator assumes that the entire scene is filled with a homogeneos medium.

class ParticipatingMediaIntegrator : public Integrator
{
public:
    ParticipatingMediaIntegrator(Bounds2i bounds, Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit)
        : Integrator(bounds, s, sampler, recursionLimit)
    {}

    // Evaluate the energy transmitted along the ray back to its origin using multiple importance sampling
    virtual Color3f Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const;

    Color3f getLightingBasedColor(const Scene &scene, std::shared_ptr<Sampler> sampler, int &numLights,
                                  Point3f &point, float &offset, float &stepsize, const Ray &ray) const;
    Color3f DirectLightingLISRayMarched(const Scene &scene, std::shared_ptr<Sampler> sampler, int &numLights,
                                     Point3f &point, float &offset, float &stepsize) const;
    Color3f IndirectLightingRayMarched(const Scene &scene, std::shared_ptr<Sampler> sampler, int &numLights,
                                       Point3f &point, float &offset, float &stepsize, const Ray &ray) const;

    bool RussianRoulette(Color3f &energy, float probability, int depth) const;
    float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf) const;
    float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf) const;
};


