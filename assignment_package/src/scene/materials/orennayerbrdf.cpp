#include "orennayerbrdf.h"
#include <warpfunctions.h>
#include <globals.h>

OrenNayerBrdf::OrenNayerBrdf(const Color3f &R, float sigma):
    BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), R(R)
{
    sigma = sigma * (Pi/180.0f);
    float sigma2 = sigma*sigma;
    A = 1.0f - (sigma2 / (2.0f * (sigma2 + 0.33f)));
    B = 0.45f * sigma2 / (sigma2 + 0.09f);
}

Color3f OrenNayerBrdf::f(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO
    float sinThetaI = std::sqrt(1.0f - CosTheta(wi)*CosTheta(wi));
    float sinThetaO = std::sqrt(1.0f - CosTheta(wo)*CosTheta(wo));

    //compute cosine term
    float maxCos = 0.0f;
    if(sinThetaI > 0.0001f && sinThetaO > 0.0001f)
    {
        float cosPhiI = std::acos(wi.x);
        float cosPhiO = std::acos(wo.x);
        float sinPhiI = std::sqrt(1.0f - cosPhiI*cosPhiI);
        float sinPhiO = std::sqrt(1.0f - cosPhiO*cosPhiO);
        float dCos = cosPhiI * cosPhiO + sinPhiI * sinPhiO;
        maxCos = std::max(0.0f, dCos);
    }

    //compute sine and tangent terms
    float sinAlpha, tanBeta;
    if(AbsCosTheta(wi) > AbsCosTheta(wo))
    {
        sinAlpha = sinThetaO;
        tanBeta = sinThetaI/AbsCosTheta(wi);
    }
    else
    {
        sinAlpha = sinThetaI;
        tanBeta = sinThetaO/AbsCosTheta(wo);
    }

    return R * InvPi * (A + B * maxCos * sinAlpha * tanBeta);
}

Color3f OrenNayerBrdf::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                        Float *pdf, BxDFType *sampledType) const
{
    //TODO
    *wi = WarpFunctions::squareToHemisphereCosine(u);
    if(wo.z < 0.0f)
    {
        wi->z *= -1.0f;
    }
    *pdf = Pdf(wo, *wi);

    return f(wo, *wi);
}

float OrenNayerBrdf::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    //TODO
    return AbsCosTheta(wi)*InvPi; //used in sampling as well so cant
    //just assume that the cos theta will come from the attenuation
    //cos theta in the light transport equation
}
