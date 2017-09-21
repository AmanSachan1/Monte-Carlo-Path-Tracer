#include "homogeneousmedia.h"

float HomogeneousMedia::Density(Vector3f pos)
{
    Noise n = Noise();
    return n.getNoise(pos);
}

float HomogeneousMedia::PhaseFunction(const Vector3f &wo, const Vector3f &wi)
{
    //implementing Henyey-Greenstein Anisotropic scattering
    float dot = glm::dot(wo, wi);
    float cosTheta = glm::cos(dot);

    float numerator = 1.0f - g*g;
    float denominator = 4*Pi*(glm::pow( ( 1+g*g-2*g*cosTheta ), 1.5f) );
    return numerator/denominator;
}

Color3f HomogeneousMedia::RetrieveColor()
{
    return Color3f(0.1f, 0.1f, 0.1f); //purple for now
}

Color3f HomogeneousMedia::EmittedLight()
{
    return Color3f(0.0f, 0.0f, 0.0f); //black for now
}

float HomogeneousMedia::Transmittance(Vector3f pos1, Vector3f pos2)
{
    float density = 0.03;//Density(pos);
    float power = -sigmaT*density*glm::distance(pos1,pos2);
    return exp(power);
}
