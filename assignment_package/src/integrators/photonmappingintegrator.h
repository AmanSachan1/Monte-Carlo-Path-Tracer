#pragma once
#include "integrator.h"
#include "directlightingintegrator.h"
#include <scene/lights/light.h>

//Similar to the FullLighting Integrator except that it uses PHOTON MAPPING to achieve convergence faster via cached photons.
//Photons are stored in Indirect, Direct, and Caustic Maps in a preprocess step in myGL.

class PhotonMappingIntegrator : public Integrator
{
public:
    PhotonMappingIntegrator(Bounds2i bounds, Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit)
        : Integrator(bounds, s, sampler, recursionLimit)
    {}

    // Evaluate the energy transmitted along the ray back to its origin using multiple importance sampling
    virtual Color3f Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const;

    bool RussianRoulette(Color3f &energy, float probability, int depth) const;
    float BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf) const;
    float PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf) const;
};
