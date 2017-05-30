#include "heterogeneousmedia.h"

float HeterogeneousMedia::Density(Vector3f pos)
{
    Noise n = Noise();
    return n.getNoise(pos);
}

float HeterogeneousMedia::PhaseFunction(const Vector3f &wo, const Vector3f &wi)
{
    //implementing Henyey-Greenstein Anisotropic scattering
    float scatteringAngle = 0.0f;
    float dot = glm::dot(wo, wi);
    float cosTheta = glm::cos(dot);

    float numerator = 1.0f - g*g;
    float denominator = 4*Pi*(glm::pow( ( 1+g*g-2*g*cosTheta ), 1.5f) );
    scatteringAngle = numerator/denominator;
}

Color3f HeterogeneousMedia::RetrieveColor()
{
    return Color3f(1.0f, 0.0f, 1.0f); //purple for now
}

Color3f HeterogeneousMedia::EmittedLight()
{
    return Color3f(0.0f, 0.0f, 0.0f); //black for now
}

float HeterogeneousMedia::Transmittance(Vector3f pos1, Vector3f pos2)
{
    return 1.0f;
}
