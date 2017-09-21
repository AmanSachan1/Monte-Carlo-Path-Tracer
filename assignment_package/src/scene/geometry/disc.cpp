#include "disc.h"
#include "warpfunctions.h"

float Disc::Area() const
{
    glm::vec3 scale = transform.getScale();
    float r1 = scale.x;
    float r2 = scale.y;
    return Pi*(r1*r2);
}

Bounds3f Disc::WorldBound() const
{
    Bounds3f b = Bounds3f();

    b.min = Point3f(-1.0f,-1.0f,0.0f);
    b.max = Point3f(1.0f,1.0f,0.0f);

    b.Apply(transform);

    return b;
}


Intersection Disc::Sample(const Point2f &xi, Float *pdf) const
{
    Point3f point = WarpFunctions::squareToDiskConcentric(xi);

    Intersection intersection = Intersection();

    intersection.normalGeometric = glm::normalize((transform.invTransT()*Normal3f(0,0,1)));
    intersection.point = glm::vec3(transform.T()*glm::vec4(point,1.0f));

    return intersection;
}

bool Disc::Intersect(const Ray &ray, Intersection *isect) const
{
    //Transform the ray
    Ray r_loc = ray.GetTransformedCopy(transform.invT());

    //Ray-plane intersection
    float t = glm::dot(glm::vec3(0,0,1), (glm::vec3(0.5f, 0.5f, 0) - r_loc.origin)) / glm::dot(glm::vec3(0,0,1), r_loc.direction);
    Point3f P = Point3f(t * r_loc.direction + r_loc.origin);
    //Check that P is within the bounds of the disc (not bothering to take the sqrt of the dist b/c we know the radius)
    float dist2 = (P.x * P.x + P.y * P.y);
    if(t > 0 && dist2 <= 1.f)
    {
        InitializeIntersection(isect, t, P);
        return true;
    }
    return false;
}

void Disc::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const
{
    *nor = glm::normalize(transform.invTransT() * Normal3f(0,0,1));
    *tan = glm::vec3(glm::normalize(transform.T() * glm::vec4(1,0,0,0)));
    *bit = glm::vec3(glm::normalize(transform.T() * glm::vec4(0,1,0,0)));
}


Point2f Disc::GetUVCoordinates(const Point3f &point) const
{
    return glm::vec2((point.x + 1)/2.f, (point.y + 1)/2.f);
}
