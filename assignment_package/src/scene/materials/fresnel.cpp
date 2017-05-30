#include "fresnel.h"

Color3f FresnelDielectric::Evaluate(float cosThetaI) const
{
    float temp_cosThetaI = glm::clamp(cosThetaI, -1.0f, 1.0f);
    float temp_etaI = etaI;
    float temp_etaT = etaT;

    bool entering = temp_cosThetaI > 0.0f;
    if(!entering)
    {
        std::swap(temp_etaI, temp_etaT);
        temp_cosThetaI = std::abs(cosThetaI);
    }

    float sinThetaI = std::sqrt(std::max( 0.0f, (1.0f - temp_cosThetaI*temp_cosThetaI) ));
    float sinThetaT = temp_etaI/temp_etaT * sinThetaI;

    if(sinThetaT >= 1.0f)
    {
        return Color3f(1.0f);
    }

    float cosThetaT = std::sqrt(std::max( 0.0f, (1.0f - sinThetaT*sinThetaT) ));

    float Rparallel = ((temp_etaT * temp_cosThetaI) - (temp_etaI * cosThetaT))/
                        ((temp_etaT * temp_cosThetaI) + (temp_etaI * cosThetaT));

    float Rperpendicular = ((temp_etaI * temp_cosThetaI) - (temp_etaT * cosThetaT))/
                        ((temp_etaI * temp_cosThetaI) + (temp_etaT * cosThetaT));

    Color3f color = Color3f((Rparallel*Rparallel + Rperpendicular*Rperpendicular)/2.0f);

    return color;
}

Color3f FresnelConductor::Evaluate(float cosThetaI) const
{
    float temp_cosThetaI = glm::clamp(cosThetaI, -1.0f, 1.0f);
    Color3f eta = Color3f(etaT / etaI);
    Color3f etak = Color3f(k / etaI);

    Float cosThetaI2 = temp_cosThetaI * temp_cosThetaI;
    Float sinThetaI2 = 1.0f - cosThetaI2;
    Color3f eta2 = eta * eta;
    Color3f etak2 = etak * etak;

    Color3f t0 = eta2 - etak2 - sinThetaI2;
    Color3f a2plusb2 = Color3f( std::sqrt(t0.x * t0.x + 4.0f * eta2.x * etak2.x),
                                std::sqrt(t0.y * t0.y + 4.0f * eta2.y * etak2.y),
                                std::sqrt(t0.z * t0.z + 4.0f * eta2.z * etak2.z));
    Color3f t1 = a2plusb2 + cosThetaI2;
    Color3f a = Color3f( std::sqrt(0.5f * (a2plusb2.x + t0.x)),
                         std::sqrt(0.5f * (a2plusb2.y + t0.y)),
                         std::sqrt(0.5f * (a2plusb2.z + t0.z)));
    Color3f t2 = 2.0f * cosThetaI * a;
    Color3f Rs = (t1 - t2) / (t1 + t2);

    Color3f t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
    Color3f t4 = t2 * sinThetaI2;
    Color3f Rp = Rs * (t3 - t4) / (t3 + t4);

    return 0.5f * (Rp + Rs);
}
