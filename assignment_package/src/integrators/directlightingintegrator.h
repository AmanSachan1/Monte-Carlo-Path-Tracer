#pragma once
#include "integrator.h"

//As the name says, for a sampled ray it returns a color corresponding to the direct light falling on the surface;
//It disregards global illumination and recursive bouncing of the light ray.

//if one were to analyse the integrator from the perspective of the Light Transport Equation, this integrator
//uses a non-recursive version of the Li term (returns a color based on a ray shot to a light source from the
//point of intersection with the scene.

class DirectLightingIntegrator : public Integrator
{
public:
    DirectLightingIntegrator(Bounds2i bounds, Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit)
        : Integrator(bounds, s, sampler, recursionLimit)
    {}

    // Evaluate the energy transmitted along the ray back to its origin, which can only be the camera in
    // a direct lighting integrator (only rays from light sources are considered)
    virtual Color3f Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const;
};
