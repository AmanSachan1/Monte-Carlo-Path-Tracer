#include <raytracing/intersection.h>

Intersection::Intersection():
    point(Point3f(0)),
    normalGeometric(Normal3f(0)),
    uv(Point2f(0)),
    t(-1),
    objectHit(nullptr),
    bsdf(nullptr),
    tangent(0.f), bitangent(0.f)
{}

bool Intersection::ProduceBSDF()
{
    return objectHit->ProduceBSDF(this);
}

Color3f Intersection::Le(const Vector3f &wo) const
{
    const AreaLight* light = objectHit->GetAreaLight();
    return light ? light->L(*this, wo) : Color3f(0.f);
}

Ray Intersection::SpawnRay(const Vector3f &d) const
{
    Vector3f originOffset = normalGeometric * RayEpsilon;
    // Make sure to flip the direction of the offset so it's in
    // the same general direction as the ray direction
    originOffset = (glm::dot(d, normalGeometric) > 0) ? originOffset : -originOffset;
    Point3f o(this->point + originOffset);
    return Ray(o, d);
}
