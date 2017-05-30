#include "lambertbrdf.h"
#include <warpfunctions.h>

Color3f LambertBRDF::f(const Vector3f &wo, const Vector3f &wi) const
{
    return R*InvPi;
}

Color3f LambertBRDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &u,
                        Float *pdf, BxDFType *sampledType) const
{
    *wi = WarpFunctions::squareToHemisphereCosine(u);

    if(wo.z < 0.0f)
    {
        wi->z *= -1.0f;
    }
    *pdf = Pdf(wo, *wi);

    return f(wo, *wi);
}

float LambertBRDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
//    if(SameHemisphere(wo, wi))
//    {
//        return AbsCosTheta(wi)*InvPi;
//    }
//    else
//    {
//        return 0.0f;
//    }

    return AbsCosTheta(wi)*InvPi; //used in sampling as well so cant
    //just assume that the cos theta will come from the attenuation
    //cos theta in the light transport equation
}
