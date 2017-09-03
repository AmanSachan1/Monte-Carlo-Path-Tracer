#include "sphere.h"
#include <iostream>
#include <math.h>
#include <warpfunctions.h>

float Sphere::Area() const
{
    return 4.f * Pi * transform.getScale().x * transform.getScale().x; // We're assuming uniform scale
}

Point3f Sphere::getPointOnSurface(const Point2f &xi) const
{
    Point3f pObj = WarpFunctions::squareToSphereUniform(xi);
    Point3f p = Point3f(transform.T() * glm::vec4(pObj.x, pObj.y, pObj.z, 1.0f));
    p += glm::normalize( transform.invTransT() *pObj ) * RayEpsilon;

    return p;
}

void Sphere::ComputeTBN(const Point3f& P, Normal3f* nor, Vector3f* tan, Vector3f* bit) const
{
    *nor = glm::normalize(transform.invTransT() * glm::normalize(P));
    *tan = glm::normalize(transform.T3() * glm::cross(Vector3f(0,1,0), (glm::normalize(P))));
    *bit = glm::normalize(glm::cross(*nor, *tan));
}

Bounds3f Sphere::WorldBound() const
{
    Bounds3f b = Bounds3f();

    b.min = Point3f(-1.0f,-1.0f,-1.0f);
    b.max = Point3f(1.0f,1.0f,1.0f);

    b.Apply(transform);

    return b;
}

bool Sphere::Intersect(const Ray &ray, Intersection *isect) const
{
    //Transform the ray
    Ray r_loc = ray.GetTransformedCopy(transform.invT());

    float A = pow(r_loc.direction.x, 2.f) + pow(r_loc.direction.y, 2.f) + pow(r_loc.direction.z, 2.f);
    float B = 2*(r_loc.direction.x*r_loc.origin.x + r_loc.direction.y * r_loc.origin.y + r_loc.direction.z * r_loc.origin.z);
    float C = pow(r_loc.origin.x, 2.f) + pow(r_loc.origin.y, 2.f) + pow(r_loc.origin.z, 2.f) - 1.f;//Radius is 1.f
    float discriminant = B*B - 4*A*C;
    //If the discriminant is negative, then there is no real root
    if(discriminant < 0){
        return false;
    }
    float t = (-B - sqrt(discriminant))/(2*A);
    if(t < 0)
    {
        t = (-B + sqrt(discriminant))/(2*A);
    }
    if(t >= 0)
    {
        Point3f P = glm::vec3(r_loc.origin + t*r_loc.direction);
        InitializeIntersection(isect, t, P);
        return true;
    }
    return false;
}

Point2f Sphere::GetUVCoordinates(const Point3f &point) const
{
    Point3f p = glm::normalize(point);
    float phi = atan2f(p.z, p.x);
    if(phi < 0)
    {
        phi += TwoPi;
    }
    float theta = glm::acos(p.y);
    return Point2f(1 - phi/TwoPi, 1 - theta / Pi);
}

Intersection Sphere::Sample(const Intersection &ref, const Point2f &xi, float *pdf) const
{
    Point3f center = Point3f(transform.T() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    Vector3f centerToRef = glm::normalize(center - ref.point);
    Vector3f tan, bit;

    CoordinateSystem(centerToRef, &tan, &bit);

    Point3f pOrigin;
    if(glm::dot(center - ref.point, ref.normalGeometric) > 0)
        pOrigin = ref.point + ref.normalGeometric * RayEpsilon;
    else
        pOrigin = ref.point - ref.normalGeometric * RayEpsilon;

    if(glm::distance2(pOrigin, center) <= 1.f) // Radius is 1, so r^2 is also 1
        return Sample(xi, pdf);

    float sinThetaMax2 = 1 / glm::distance2(ref.point, center); // Again, radius is 1
    float cosThetaMax = std::sqrt(glm::max((float)0.0f, 1.0f - sinThetaMax2));
    float cosTheta = (1.0f - xi.x) + xi.x * cosThetaMax;
    float sinTheta = std::sqrt(glm::max((float)0, 1.0f- cosTheta * cosTheta));
    float phi = xi.y * 2.0f * Pi;

    float dc = glm::distance(ref.point, center);
    float ds = dc * cosTheta - glm::sqrt(glm::max((float)0.0f, 1 - dc * dc * sinTheta * sinTheta));

    float cosAlpha = (dc * dc + 1 - ds * ds) / (2 * dc * 1);
    float sinAlpha = glm::sqrt(glm::max((float)0.0f, 1.0f - cosAlpha * cosAlpha));

    Vector3f nObj = sinAlpha * glm::cos(phi) * -tan + sinAlpha * glm::sin(phi) * -bit + cosAlpha * -centerToRef;
    Point3f pObj = Point3f(nObj); // Would multiply by radius, but it is always 1 in object space

    Intersection isect;

//    pObj *= radius / glm::length(pObj); // pObj is already in object space with r = 1, so no need to perform this step

    isect.point = Point3f(transform.T() * glm::vec4(pObj.x, pObj.y, pObj.z, 1.0f));
    isect.normalGeometric = transform.invTransT() * Normal3f(nObj);

    *pdf = 1.0f / (2.0f * Pi * (1 - cosThetaMax));

    return isect;
}

Intersection Sphere::Sample(const Point2f &xi, Float *pdf) const
{
    Point3f pObj = WarpFunctions::squareToSphereUniform(xi);

    Intersection it;
    it.normalGeometric = glm::normalize( transform.invTransT() *pObj );
    it.point = Point3f(transform.T() * glm::vec4(pObj.x, pObj.y, pObj.z, 1.0f));

    *pdf = 1.0f / Area();

    return it;
}
