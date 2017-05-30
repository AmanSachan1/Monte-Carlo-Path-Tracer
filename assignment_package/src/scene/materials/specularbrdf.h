#pragma once
#include "bsdf.h"
#include "fresnel.h"

class SpecularBRDF : public BxDF
{
public:
    SpecularBRDF(const Color3f &R, Fresnel* fresnel)
        : BxDF(BxDFType(BSDF_REFLECTION | BSDF_SPECULAR)), R(R), fresnel(fresnel) {}

    ~SpecularBRDF(){delete fresnel;}

    Color3f f(const Vector3f &wo, const Vector3f &wi) const;

    virtual Color3f Sample_f(const Vector3f &wo, Vector3f *wi,
                              const Point2f &sample, Float *pdf,
                              BxDFType *sampledType = nullptr) const;
    virtual float Pdf(const Vector3f &wo, const Vector3f &wi) const;


  private:
    const Color3f R; // The energy scattering coefficient of this BRDF (i.e. its color)
    const Fresnel* fresnel; // A class that will determine the reflectivity coefficient at this point
};
