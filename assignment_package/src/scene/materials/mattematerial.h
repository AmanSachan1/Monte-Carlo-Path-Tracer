#pragma once
#include "material.h"

class MatteMaterial : public Material
{
public:
    MatteMaterial(const Color3f &Kd,
                  float sigma,
                  const std::shared_ptr<QImage> &textureMap,
                  const std::shared_ptr<QImage> &normalMap)
        : Kd(Kd), sigma(sigma), textureMap(textureMap), normalMap(normalMap)
    {}

    void ProduceBSDF(Intersection *isect) const;


private:
    Color3f Kd;                    // The diffuse spectral reflection of this material.
                                   // This is just the base color of the material

    float sigma;                   // The roughness of this material (a single instance is a float). This determines whether or not
                                   // we use a Lambertian or Oren-Nayar BRDF in ComputeScatteringFunctions.
                                   // When nonzero, we use Oren-Nayar.

    std::shared_ptr<QImage> textureMap; // The color obtained from this (assuming it is non-null) is multiplied with the base material color.
    std::shared_ptr<QImage> normalMap;
};
