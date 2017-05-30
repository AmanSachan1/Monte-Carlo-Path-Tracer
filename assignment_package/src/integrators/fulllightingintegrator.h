#pragma once
#include "integrator.h"
#include "directlightingintegrator.h"
#include <scene/lights/light.h>

//As the name says, for a sampled ray it returns a color according to the Light Transport Equation in its entirety
//It takes into account global illumination and has a recursive call for the Li term

class FullLightingIntegrator : public Integrator
{
public:
    FullLightingIntegrator(Bounds2i bounds, Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit)
        : Integrator(bounds, s, sampler, recursionLimit)
    {}

    // Evaluate the energy transmitted along the ray back to its origin using multiple importance sampling
    virtual Color3f Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const;

    float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf) const;
    float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf) const;
    bool RussianRoulette(Color3f &energy, float probability, int depth) const;
};


