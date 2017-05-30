#include "microfacetbrdf.h"

Color3f MicrofacetBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    float cosThetaO = AbsCosTheta(wo);
    float cosThetaI = AbsCosTheta(wi);
    Vector3f wh = wi + wo;

    //Handle degenerate cases for microfacet reflection
    if(cosThetaI == 0.0f || cosThetaO == 0.0f)
    {
        return Color3f(0.0f);
    }
    if((wh.x == 0.0f) && (wh.y == 0.0f) && (wh.z == 0.0f))
    {
        return Color3f(0.0f);
    }

    wh = glm::normalize(wh);
    Color3f f = fresnel->Evaluate(glm::dot(wi, wh));
    return R * distribution->D(wh) * distribution->G(wo, wi) * f / (4.0f * cosThetaI * cosThetaO);
}

Color3f MicrofacetBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi, Float *pdf, BxDFType *sampledType) const
{
    //Sample the microfacet orientation wh and reflected direction wi
    Vector3f wh = distribution->Sample_wh(wo, xi);
    *wi = glm::reflect(-wo, wh); //wo should be negated, book doesn't
    if(!SameHemisphere(wo, *wi))
    {
        return Color3f(0.0f);
    }

    //Compute PDF of wi for microfacet reflection
    *pdf = distribution->Pdf(wo, wh) / (4.0f * glm::dot(wo,wh));

    return f(wo, *wi);
}

float MicrofacetBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    if(!SameHemisphere(wo, wi))
    {
        return 0.0f;
    }
    Vector3f wh = glm::normalize(wo + wi);
    return distribution->Pdf(wo, wh) / (4.0f * glm::dot(wo, wh));
}
