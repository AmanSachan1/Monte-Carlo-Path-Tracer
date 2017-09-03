#include "squareplane.h"
#include "warpfunctions.h"

float SquarePlane::Area() const
{
    glm::vec3 scale = transform.getScale();
    return scale.x*scale.y;
}

Point3f SquarePlane::getPointOnSurface(const Point2f &xi) const
{
    Point3f pObj = Point3f(xi[0]-0.5f, xi[1]-0.5f, 0.0f);
    Point3f p = Point3f(transform.T() * glm::vec4(pObj.x, pObj.y, pObj.z, 1.0f));
    p += glm::normalize((transform.invTransT()*Normal3f(0,0,1))) * RayEpsilon;

    return p;
}

Bounds3f SquarePlane::WorldBound() const
{
    Bounds3f b = Bounds3f();

    b.min = Point3f(-0.5f,-0.5f,0.0f);
    b.max = Point3f(0.5f,0.5f,0.0f);

    b.Apply(transform);

    return b;
}

Intersection SquarePlane::Sample(const Point2f &xi, Float *pdf) const
{
    Point3f point = Point3f(xi[0]-0.5f, xi[1]-0.5f, 0.0f);

    Intersection intersection = Intersection();

    intersection.normalGeometric = glm::normalize((transform.invTransT()*Normal3f(0,0,1)));
    intersection.point = glm::vec3(transform.T()*glm::vec4(point,1.0f));

    return intersection;
}

bool SquarePlane::Intersect(const Ray &ray, Intersection *isect) const
{
    //Transform the ray
    Ray r_loc = ray.GetTransformedCopy(transform.invT());

    //Ray-plane intersection
    float t = glm::dot(glm::vec3(0,0,1), (glm::vec3(0.5f, 0.5f, 0) - r_loc.origin)) / glm::dot(glm::vec3(0,0,1), r_loc.direction);
    Point3f P = Point3f(t * r_loc.direction + r_loc.origin);
    //Check that P is within the bounds of the square
    if(t > 0 && P.x >= -0.5f && P.x <= 0.5f && P.y >= -0.5f && P.y <= 0.5f)
    {
        InitializeIntersection(isect, t, P);
        return true;
    }
    return false;
}

void SquarePlane::ComputeTBN(const Point3f &P, Normal3f *nor, Vector3f *tan, Vector3f *bit) const
{
    *nor = glm::normalize(transform.invTransT() * Normal3f(0,0,1));
    *tan = glm::normalize(transform.T3() * Normal3f(1,0,0));
    *bit = glm::normalize(glm::cross(*nor, *tan));
}


Point2f SquarePlane::GetUVCoordinates(const Point3f &point) const
{
    return Point2f(point.x + 0.5f, point.y + 0.5f);
}
