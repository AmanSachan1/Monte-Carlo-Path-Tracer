#include "medium.h"

// This should be decreasing as p2 gets farther and farther from p1
float Medium::Transmittance(Point3f p1, Point3f p2)
{
//    float density = 0.001275f;  // Water
//    float density = 0.03f;
//    float density = 0.015f;
//    float density = .75f;
    float density = 1.f;
    float exponent = -sigmaT * density * glm::distance(p1, p2);

    return expf(exponent);
}

float Medium::PhaseHG(Vector3f wo, Vector3f wi)
{
    float dot = glm::dot(wo, wi);
    float cosTheta = glm::cos(dot);

    float num = 1.f - (g * g);
    float denom = 4 * Pi * (glm::pow(1 + g * g - 2 * g * cosTheta, 1.5f));

    return num / denom;
}
