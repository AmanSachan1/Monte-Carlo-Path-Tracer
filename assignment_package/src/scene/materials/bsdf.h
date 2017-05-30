#pragma once
#include <globals.h>
#include <raytracing/intersection.h>

enum BxDFType {
    BSDF_REFLECTION = 1 << 0,   // This BxDF handles rays that are reflected off surfaces
    BSDF_TRANSMISSION = 1 << 1, // This BxDF handles rays that are transmitted through surfaces
    BSDF_DIFFUSE = 1 << 2,      // This BxDF represents diffuse energy scattering, which is uniformly random
    BSDF_GLOSSY = 1 << 3,       // This BxDF represents glossy energy scattering, which is biased toward certain directions
    BSDF_SPECULAR = 1 << 4,     // This BxDF handles specular energy scattering, which has no element of randomness
    BSDF_ALL = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION | BSDF_TRANSMISSION
};

class Intersection;
class BxDF;

class BSDF
{
public:
    //DELETEME The PBRT BSDF stores normal and tangent data from the intersection that created it
    // Our path tracer will pre-transform vectors before they've been passed into f()

    BSDF(const Intersection& isect, float eta = 1);

    ~BSDF();

    void Add(BxDF *b){if(numBxDFs < MaxBxDFs){bxdfs[numBxDFs++] = b;}}


    // Compute the f() result of all of our BxDFs which match the BxDFType indicated
    // Returns the sum of these f()s
    // IMPORTANT: Convert woW and wiW from world space to tangent space
    // before passing them to BxDF::f()!
    Color3f f(const Vector3f &woW, const Vector3f &wiW, BxDFType flags = BSDF_ALL) const;

    // Invokes the Sample_f() of one of our BxDFs, chosen at random from the set
    // of BxDFs that match the BxDFType indicated.
    // Additionally invokes the f() of all BxDFs after using Sample_f to generate
    // a ray direction.
    // Refer to the .cpp file for a more detailed function description.
    Color3f Sample_f(const Vector3f &woW, Vector3f *wiW, const Point2f &xi,
                     float *pdf, BxDFType type = BSDF_ALL,
                     BxDFType *sampledType = nullptr) const;

    // Computes the average PDF of all BxDFs that match the given BxDFType
    // i.e. sum each BxDF's Pdf() result and divide by the number of BxDFs
    // that match _flags_.
    // Note that _woW_ and _wiW_ are given in world space, and must
    // be converted to tangent space before being passed to BxDF::Pdf
    float Pdf(const Vector3f &woW, const Vector3f &wiW,
              BxDFType flags = BSDF_ALL) const;

    // Compute the number of BxDFs that match the input flags.
    int BxDFsMatchingFlags(BxDFType flags) const;

    void UpdateTangentSpaceMatrices(const Normal3f& n, const Vector3f& t, const Vector3f b);

    glm::mat3 worldToTangent; // Transforms rays from world space into tangent space,
                              // where the surface normal is always treated as (0, 0, 1)
    glm::mat3 tangentToWorld; // Transforms rays from tangent space into world space.
                              // This is the inverse of worldToTangent (incidentally, inverse(worldToTangent) = transpose(worldToTangent))

    Normal3f normal;          // May be the geometric normal OR the shading normal at the point of intersection.
                              // If the Material that created this BSDF had a normal map, then this will be the latter.

    float eta; // The ratio of indices of refraction at this surface point. Irrelevant for opaque surfaces.

private:
    int numBxDFs; // How many BxDFs this BSDF currently contains (init. 0)
    const static int MaxBxDFs = 8; // How many BxDFs a single BSDF can contain
    BxDF* bxdfs[MaxBxDFs]; // The collection of BxDFs contained in this BSDF
};

class BxDF
{
public:
    BxDF(BxDFType type) : type(type) {}
    virtual ~BxDF() {}

    virtual Color3f f(const Vector3f &woW, const Vector3f &wiW) const = 0;

    // Generates a uniformly random sample on the hemisphere
    // and evaluates f() on it.
    // Specific BxDF subclasses can (and should) override this.
    virtual Color3f Sample_f(const Vector3f &wo, Vector3f *wi,
                             const Point2f &xi, Float *pdf,
                             BxDFType *sampledType = nullptr) const;

    // Evaluates the PDF for uniformly sampling a hemisphere.
    // Specific BxDF subclasses can (and should) override this.
    virtual float Pdf(const Vector3f &wo, const Vector3f &wi) const;

                                            //Bitwise AND. This is not a reference.
    bool MatchesFlags(BxDFType t) const { return (type & t) == type; }

    const BxDFType type;
};

inline int BSDF::BxDFsMatchingFlags(BxDFType flags) const
{
    int num = 0;
    for (int i = 0; i < numBxDFs; ++i)
        if (bxdfs[i]->MatchesFlags(flags)) ++num;
    return num;
}
