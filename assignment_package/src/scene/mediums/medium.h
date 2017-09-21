#pragma once

#include "samplers/sampler.h"
#include <math.h>

class Medium
{
public:
    Medium(float sa, float ss) : sigmaA(sa), sigmaS(ss)
    {
        sigmaT = sigmaA + sigmaS;
    }
    virtual ~Medium(){}

    virtual float Density(Vector3f pos) = 0;
    virtual float PhaseFunction(const Vector3f &wo, const Vector3f &wi) = 0;
    virtual Color3f RetrieveColor() = 0;
    virtual Color3f EmittedLight() = 0;
    virtual float Transmittance(Vector3f pos1, Vector3f pos2) = 0;

    //member variables
    float sigmaA; //Absorption Coefficient
    float sigmaS; //Scattering Coefficient
    float sigmaT; //Extinction Coefficient
};
