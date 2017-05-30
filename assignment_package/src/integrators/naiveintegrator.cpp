#include "naiveintegrator.h"

Color3f NaiveIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    Vector3f woW = -ray.direction;
    Vector3f wiW;
    float pdf;
    Point2f xi = sampler->Get2D();
    BxDFType type = BSDF_ALL;
    BxDFType sampledType;

    Color3f finalcolor = Color3f(0.0f);
    Intersection intersection = Intersection();
    bool objhit = scene.Intersect(ray, &intersection);
    Color3f f = Color3f(0.0f);

    if(!objhit)
    {
        return Color3f(0.0f);
    }

    if(intersection.objectHit->material == nullptr)
    {
        return intersection.Le(woW);
    }

    if(depth == 0)
    {
        return finalcolor;
    }

    intersection.ProduceBSDF();

    f += intersection.bsdf->Sample_f(woW, &wiW, xi, &pdf, type, &sampledType);
    if(pdf == 0.f)
    {
        return intersection.Le(woW);
    }

    Ray n_ray = intersection.SpawnRay(wiW);

    float absDot = AbsDot(wiW, intersection.normalGeometric);

    finalcolor = (intersection.Le(woW) + f * Li(n_ray, scene, sampler, depth-1) * absDot) / pdf;

    return finalcolor;
}
