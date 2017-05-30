#pragma once
#include "material.h"

class GlassMaterial : public Material
{
public:
    GlassMaterial(const Color3f &Kr, const Color3f& Kt,
                  float indexOfRefraction,
                  const std::shared_ptr<QImage> &textureMapRefl,
                  const std::shared_ptr<QImage> &textureMapTransmit,
                  const std::shared_ptr<QImage> &normalMap)
        : Kr(Kr), Kt(Kt), indexOfRefraction(indexOfRefraction),
          textureMapRefl(textureMapRefl),
          textureMapTransmit(textureMapTransmit),
          normalMap(normalMap)
    {}

    void ProduceBSDF(Intersection *isect) const;


private:
    Color3f Kr;                    // The spectral reflection of this material.
                                   // This is just the base color of the reflections

    Color3f Kt;                    // The color by which rays are attenuated when
                                   // transmitted through this material.

    float indexOfRefraction;       // The IoR of this glass. We assume the IoR of
                                   // any external medium is that of a vacuum: 1.0

    std::shared_ptr<QImage> textureMapRefl;
    std::shared_ptr<QImage> textureMapTransmit;
    std::shared_ptr<QImage> normalMap;
};
