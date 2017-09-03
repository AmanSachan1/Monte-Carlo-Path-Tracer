#pragma once

#include "raytracing/ray.h"
#include "samplers/sampler.h"
#include "warpfunctions.h"
#include "samplers/sampler.h"

// Assume Homogenous Medium

class Medium
{
public:
    Medium(float sigmaA, float sigmaS, float g, Color3f color) :
        sigmaA(sigmaA), sigmaS(sigmaS), sigmaT(sigmaA + sigmaS),
        g(g), color(color), sigma_a(Color3f(sigmaA)), sigma_s(Color3f(sigmaS)), sigma_t(sigma_a + sigma_s) {}
    ~Medium() {}

    float Transmittance(Point3f p1, Point3f p2);
    float PhaseHG(Vector3f wo, Vector3f wi);

    Color3f sigma_a, sigma_s, sigma_t;

    // Albedo
    Color3f color;
    // Absorbtion coefficient
    // PDF that describes light absorbed per unit distance traveled in medium
    // Not constrained to [0,1]
    // Reduces the radiance along a ray
    float sigmaA;
    // Scattering coefficient
    // Probability of scattering at a scattering event
    float sigmaS;
    // Attenutation coefficient
    // sigmaA + sigmaS
    float sigmaT;
    // Assymetry parameter
    // controls the scattering light distribution
    float g;
    // Density
    float density = 0.005f;

};
