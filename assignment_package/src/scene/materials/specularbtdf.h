#pragma once
#include "bsdf.h"
#include "fresnel.h"

class SpecularBTDF : public BxDF
{
public:
    SpecularBTDF(const Color3f &T, float etaA, float etaB, Fresnel* fresnel)
        : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)),
          T(T), etaA(etaA), etaB(etaB), fresnel(fresnel) {}

    ~SpecularBTDF(){delete fresnel;}

    Color3f f(const Vector3f &wo, const Vector3f &wi) const;

    virtual Color3f Sample_f(const Vector3f &wo, Vector3f *wi,
                              const Point2f &sample, Float *pdf,
                              BxDFType *sampledType = nullptr) const;
    virtual float Pdf(const Vector3f &wo, const Vector3f &wi) const;


  private:
    const Color3f T; // The energy scattering coefficient of this BTDF (i.e. its color)
    float etaA, etaB;
    const Fresnel* fresnel; // A class that will determine the reflectivity coefficient at this point
};
