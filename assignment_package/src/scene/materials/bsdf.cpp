#include "bsdf.h"
#include <warpfunctions.h>

BSDF::BSDF(const Intersection& isect, float eta /*= 1*/)
//Properly set worldToTangent and tangentToWorld
    : worldToTangent(Matrix3x3()),
      tangentToWorld(Matrix3x3()),
      normal(isect.normalGeometric),
      eta(eta),
      numBxDFs(0),
      bxdfs{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}
{
    UpdateTangentSpaceMatrices(isect.normalGeometric, isect.tangent, isect.bitangent);
}

BSDF::~BSDF() {
    for(int i=0; i<numBxDFs ;i++)
    {
        delete bxdfs[i];
    }
}

void BSDF::UpdateTangentSpaceMatrices(const Normal3f& n, const Vector3f& t, const Vector3f b)
{
    //Update worldToTangent and tangentToWorld based on the normal, tangent, and bitangent
    tangentToWorld = glm::mat3(t,b,n);
    worldToTangent = glm::transpose(tangentToWorld);
}


Color3f BSDF::f(const Vector3f &woW, const Vector3f &wiW, BxDFType flags /*= BSDF_ALL*/) const
{
    glm::vec3 wo = worldToTangent * woW;
    glm::vec3 wi = worldToTangent * wiW;

    bool reflect = glm::dot(wiW, normal) * glm::dot(woW, normal) > 0.0f;
    Color3f finalColor = glm::vec3(0.0f);
    for(int i=0; i<numBxDFs ;i++)
    {
        if(bxdfs[i]->MatchesFlags(flags) &&
                ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
                (!reflect && (bxdfs[i]->type & BSDF_TRANSMISSION))))
        {
            finalColor += bxdfs[i]->f(wo,wi);
        }
    }
    return finalColor;
}

// Use the input random number _xi_ to select
// one of our BxDFs that matches the _type_ flags.

// After selecting our random BxDF, rewrite the first uniform
// random number contained within _xi_ to another number within
// [0, 1) so that we don't bias the _wi_ sample generated from
// BxDF::Sample_f.

// Convert woW and wiW into tangent space and pass them to
// the chosen BxDF's Sample_f (along with pdf).
// Store the color returned by BxDF::Sample_f and convert
// the _wi_ obtained from this function back into world space.

// Iterate over all BxDFs that we DID NOT select above (so, all
// but the one sampled BxDF) and add their PDFs to the PDF we obtained
// from BxDF::Sample_f, then average them all together.

// Finally, iterate over all BxDFs and sum together the results of their
// f() for the chosen wo and wi, then return that sum.

Color3f BSDF::Sample_f(const Vector3f &woW, Vector3f *wiW, const Point2f &xi,
                       float *pdf, BxDFType type, BxDFType *sampledType) const
{
    //u is a samplepoint

    //-------------------------------------------------------------------------
    //choose which BxDF to sample
    int matchingComps = BxDFsMatchingFlags(type);
    if(matchingComps == 0)
    {
        *pdf = 0; //because we don't want to sample it???
        return Color3f(0.0f);
    }
    int comp = std::min((int)std::floor(xi[0]*matchingComps), matchingComps - 1);
    // random bxdf choosing

    BxDF *bxdf = nullptr;
    int count = comp;

    for(int i=0; i<numBxDFs ;++i)
    {       
        if(bxdfs[i]->MatchesFlags(type) && count-- == 0)
            // count is only decremented when a bxdfs[i] == mathcing flag
        {
            bxdf = bxdfs[i];
            break;
        }
    }

    //-------------------------------------------------------------------------
    //Remap BxDF sample u to [0,1)
    Point2f uRemapped = Point2f(std::min(xi[0] * matchingComps - comp, OneMinusEpsilon), xi[1]);
    //so that we can sample the entire hemisphere and not just a subsection of it;
    //xi[1] is independent of the bxdf, and so the bxdf cant bias it

    //-------------------------------------------------------------------------
    //Sample chosen BxDF
    Vector3f wo = worldToTangent * woW;
    Vector3f wi = Vector3f (0.0f);
    *pdf = 0.0f;
    if(sampledType)
    {
        //this is because its a recursive call; we are shooting a ray that bounces off
        //many materials on its, journey, so if it was sampled before, then sampledType
        //will exist and we have to reequate it to the type of the material hit
        *sampledType = type;
    }

    Color3f Color = bxdf->Sample_f(wo, &wi, uRemapped, pdf, sampledType);
    if(*pdf == 0)
    {
//        std::cout << "*pdf == 0. Sample_f gave us black color and 0 PDF!" << std::endl;
//        std::cout << "XI: " << glm::to_string(xi) << std::endl;
//        std::cout << "wi: " << glm::to_string(wi) << std::endl;
        return Color3f(0.0f);
    }
    *wiW = tangentToWorld * wi;

    //-------------------------------------------------------------------------
    //compute overall pdf with all matching BxDFs

    if(!(bxdf->type & BSDF_SPECULAR) && (matchingComps>1))
    {
        for(int i=0; i<numBxDFs; ++i)
        {
            if(bxdfs[i] != bxdf && bxdfs[i]->MatchesFlags(type))
            {
                //overall pdf!!!! so get all pdfs for the
                //different bxdfs and average it out for the bsdf
                *pdf += bxdfs[i]->Pdf(wo, wi);
            }
        }
    }
    if(matchingComps > 1)
    {
        *pdf /= matchingComps;
    }

    //-------------------------------------------------------------------------
    //compute value of BSDF for sampled direction
    if(!(bxdf->type & BSDF_SPECULAR) && (matchingComps>1) )
    {
        bool reflect = (glm::dot(*wiW, normal) * glm::dot(woW, normal)) > 0;
        Color = Color3f(0.0); //because the material is reflective or
                             //transmissive so doesn't have its own color
        for(int i=0; i<numBxDFs; ++i)
        {
            if(bxdfs[i]->MatchesFlags(type) &&
                    ((reflect && (bxdfs[i]->type & BSDF_REFLECTION)) ||
                     (!reflect && (bxdfs[i]->type &BSDF_TRANSMISSION))))
            {
                Color += bxdfs[i]->f(wo,wi);
            }
        }
    }

    return Color;
}


float BSDF::Pdf(const Vector3f &woW, const Vector3f &wiW, BxDFType flags) const
{
    float pdf = 0.0f;
    for(int i=0; i<numBxDFs; ++i)
    {
        if(bxdfs[i]->MatchesFlags(flags))
        {
            pdf += bxdfs[i]->Pdf(woW,wiW);
        }
    }
    return pdf;
}

Color3f BxDF::Sample_f(const Vector3f &wo, Vector3f *wi, const Point2f &xi,
                       Float *pdf, BxDFType *sampledType) const
{
    //cosine weighted version of sample_f
    *wi = WarpFunctions::squareToHemisphereCosine(xi);
//    *wi = WarpFunctions::squareToHemisphereUniform(xi);

    if(wo.z <0)
    {
        wi->z *= -1;
    }
    *pdf = Pdf(wo, *wi);
    return f(wo, *wi);
}

// The PDF for uniform hemisphere sampling
float BxDF::Pdf(const Vector3f &wo, const Vector3f &wi) const
{
    return SameHemisphere(wo, wi) ? Inv2Pi : 0;
}
