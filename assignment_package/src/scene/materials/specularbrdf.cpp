#include "specularbrdf.h"

Color3f SpecularBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    return Color3f(0.f); //This is because we will assume that ωi has a
    //zero percent chance of being randomly set to the exact mirror of ωo by
    //any other BxDF's Sample_f, hence a PDF of zero.
}


float SpecularBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return 0.0f; //This is because we will assume that ωi has a
    //zero percent chance of being randomly set to the exact mirror of ωo by
    //any other BxDF's Sample_f, hence a PDF of zero.
}

Color3f SpecularBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const
{
    *wi = Vector3f(-wo.x, -wo.y, wo.z);
    *pdf = 1.0f;
    Color3f color = fresnel->Evaluate(wi->z) * R / AbsCosTheta(*wi);
    return color;
}
