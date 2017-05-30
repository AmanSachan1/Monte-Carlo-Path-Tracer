#include "plasticmaterial.h"
#include "lambertbrdf.h"
#include "fresnel.h"
#include "microfacetbrdf.h"
#include "microfacet.h"

void PlasticMaterial::ProduceBSDF(Intersection *isect) const
{
    // Important! Must initialize the intersection's BSDF!
    isect->bsdf = std::make_shared<BSDF>(*isect);

    if(this->normalMap)
    {
        isect->bsdf->normal = isect->bsdf->tangentToWorld *  Material::GetImageColor(isect->uv, this->normalMap.get());
        //Update bsdf's TBN matrices to support the new normal
        Vector3f tangent, bitangent;
        CoordinateSystem(isect->bsdf->normal, &tangent, &bitangent);
        isect->bsdf->UpdateTangentSpaceMatrices(isect->bsdf->normal, tangent, bitangent);
    }

    // Lambertian BRDF component
    Color3f colorDiffuse = Kd;
    if(this->diffuseColorMap)
    {
        colorDiffuse *= Material::GetImageColor(isect->uv, this->diffuseColorMap.get());
    }
    isect->bsdf->Add(new LambertBRDF(colorDiffuse));

    // Microfacet Specular BRDF component
    float rough = roughness;
    if(this->roughnessMap)
    {
        // Gets the greyscale value of the roughness map image and attenuates it by the
        // overall roughness of this material
        Color3f roughRGB = Material::GetImageColor(isect->uv, this->roughnessMap.get());
        rough *= (0.299 * roughRGB.r + 0.587 * roughRGB.g + 0.114 * roughRGB.b);
    }
    //Convert roughness from 0,1 scale to alpha term in Trowbridge-Reitz distribution
//    rough = RoughnessToAlpha(rough);
    MicrofacetDistribution* distrib = new TrowbridgeReitzDistribution(rough, rough);

    Color3f colorSpecular = Ks;
    if(specularColorMap)
    {
        colorSpecular *= Material::GetImageColor(isect->uv, this->specularColorMap.get());
    }

    Fresnel* fresnel = new FresnelDielectric(1.5f, 1.f);

    isect->bsdf->Add(new MicrofacetBRDF(Ks, distrib, fresnel));
}
