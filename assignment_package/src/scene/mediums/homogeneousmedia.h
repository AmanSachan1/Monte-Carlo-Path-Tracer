#pragma once

#include "scene/mediums/medium.h"
#include "scene/noise.h"
#include "globals.h"

class HomogeneousMedia: public Medium
{
public:
    HomogeneousMedia( float sa, float ss, float g) : Medium(sa, ss), g(g)
    {
        sigmaT = sigmaA + sigmaS;
    }

    virtual float Density(Vector3f pos);
    virtual float PhaseFunction(const Vector3f &wo, const Vector3f &wi);
    virtual Color3f RetrieveColor();
    virtual Color3f EmittedLight();
    virtual float Transmittance(Vector3f pos1, Vector3f pos2);

    //Member Variables
    const float g;
};
