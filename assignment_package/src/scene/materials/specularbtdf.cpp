#include "specularbTdf.h"

Color3f SpecularBTDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    return Color3f(0.f);
}


float SpecularBTDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return 0.f;
}

Color3f SpecularBTDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &sample, Float *pdf, BxDFType *sampledType) const
{
    //TODO!
    //This function is a little more involved, however, as you not only must generate
    //ωi by refracting ωo, but you must also check for total internal reflection and
    //return black if it would occur. -----> We return black because in the case of total
    //internal reflection, the surface is a 100% reflective and we could sample end up
    //sampling the specularBRDF which would also give a fresnel coefficient of 1; We don't want
    // to deal with this scenario in both scenarios

    bool flag_entering = CosTheta(wo) > 0;
    float etaI = flag_entering? etaA : etaB;
    float etaT = flag_entering? etaB : etaA;

    if(!Refract(wo, Faceforward( Normal3f(0,0,1), wo), etaI/etaT, wi))
    {
        //Faceforward( Normal3f(0,0,1), wo) returns the correct normal either +n or -n
        return Color3f(0.0f);
    }

    *pdf = 1.0f;

    Color3f color = T * (Color3f(1.0) - fresnel->Evaluate(CosTheta(*wi)));

    return color/AbsCosTheta(*wi);
}
