#include "directlightingintegrator.h"

Color3f DirectLightingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    Vector3f woW = -ray.direction;
    Vector3f wiW;
    float pdf;
    Point2f xi = sampler->Get2D();
    BxDFType type = BSDF_ALL;

    Color3f finalcolor = Color3f(0.0f);
    Intersection intersection = Intersection();
    bool objhit = scene.Intersect(ray, &intersection);

    if(!objhit)
    {
        return Color3f(0.0f);
    }

    if(intersection.objectHit->material == nullptr)
    {
        return intersection.Le(woW);
    }

    int numLights = scene.lights.size();
    if(numLights == 0)
    {
        return Color3f(0.0f);
    }
    int randomLightIndex = std::min((int)std::floor(sampler->Get1D()*numLights), numLights - 1);

    Color3f Li = scene.lights.at(randomLightIndex)->Sample_Li(intersection, xi, &wiW, &pdf);
    pdf = pdf/numLights;

    if(pdf == 0.0f)
    {
        return Color3f(0.f);
    }

    intersection.ProduceBSDF();
    Color3f f = intersection.bsdf->f(woW, wiW, type);

    Ray n_ray = intersection.SpawnRay(wiW);
    float absDot = AbsDot(wiW, intersection.normalGeometric);

    //visibility test
    Intersection isx;
    objhit = scene.Intersect(n_ray, &isx);

    if(objhit)
    {
        if(isx.objectHit->areaLight != scene.lights.at(randomLightIndex))
        {
            return Color3f(0.0f);
        }
    }

    finalcolor = intersection.Le(woW) + (f * Li * absDot) / pdf;

    return finalcolor;
}
