#pragma once
#include <globals.h>

class MicrofacetDistribution {
public:
    virtual ~MicrofacetDistribution();

    // Computes the differential area of microfacets on a
    // surface that are aligned with the given surface normal
    // vector wh (the half-vector between wo and its specular
    // reflection wi)
    virtual float D(const Vector3f &wh) const = 0;

    //
    virtual float Lambda(const Vector3f &w) const = 0;

    // Computes the geometric self-shadowing and interreflection term
    float G(const Vector3f &wo, const Vector3f &wi) const {
        return 1 / (1 + Lambda(wo) + Lambda(wi));
    }

    // Samples the distribution of microfacet normals to generate one
    // about which to reflect wo to create a wi.
    virtual Vector3f Sample_wh(const Vector3f &wo, const Point2f &xi) const = 0;

    // Computes the PDF of the given half-vector normal based on the
    // given incident ray direction
    float Pdf(const Vector3f &wo, const Vector3f &wh) const;
};

class TrowbridgeReitzDistribution : public MicrofacetDistribution {
public:
    TrowbridgeReitzDistribution(float alphax, float alphay)
        : alphax(alphax), alphay(alphay) {}

    float D(const Vector3f &wh) const;
    Vector3f Sample_wh(const Vector3f &wo, const Point2f &xi) const;

private:
    float Lambda(const Vector3f &w) const;

    const float alphax, alphay;
};

// Converts a [0, 1] scalar measurement of surface roughness
// to the alpha term used in the microfacet distribution equation.
// It is not presently used in the materials we have provided you,
// but feel free to add it in if you want to see how it alters your renders.
inline float RoughnessToAlpha(float roughness) {
    //    return roughness;
    roughness = std::max(roughness, (float)1e-3);
    float x = std::log(roughness);
    return 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x +
            0.000640711f * x * x * x * x;
}
