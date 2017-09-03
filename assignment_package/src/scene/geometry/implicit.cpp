#include "implicit.h"
#include "warpfunctions.h"

Bounds3f Implicit::WorldBound() const
{
    Point3f max(5.0f,5.0f,5.0f);
    Point3f min(-5.0f,-5.0f,-5.0f);
    Bounds3f bounds(min,max);

    return bounds.Apply(this->transform);
}

float Implicit::Area() const
{
    return 0;
}

Point3f Implicit::getPointOnSurface(const Point2f &xi) const
{
    //because you would have to redefine this for every new implicit function you write
    //assume theres a bounding sphere enclosing the implicit surface
    //use the bounding sphere as the proxy for the surface
    Point3f pObj = WarpFunctions::squareToSphereUniform(xi);
    Point3f p = Point3f(transform.T() * glm::vec4(pObj.x, pObj.y, pObj.z, 1.0f));
    p += glm::normalize( transform.invTransT() *pObj ) * RayEpsilon;

    return p;
}

// Tanglecube
float sceneSDF(Point3f p)
{
    float x2 = p.x * p.x;
    float x4 = x2 * x2;
    float y2 = p.y * p.y;
    float y4 = y2 * y2;
    float z2 = p.z * p.z;
    float z4 = z2 * z2;
    float a = 0.0f;
    float b = -1.0f;
    float c = 0.5f;

    return (x4+y4+z4) + std::pow( a*(x2+y2+z2),2 ) + b*(x2+y2+z2) + c;
}

glm::vec2 Implicit::GetUVCoordinates(const glm::vec3 &) const
{
   return glm::vec2(0.f,0.f);
}

bool Implicit::Intersect(const Ray& r, Intersection* isect) const
{
    //Transform the ray
    float t = 0.f;
    float stepSize = 0.1f;
    float smallStepSize = 0.02f;
    Ray r_loc = r.GetTransformedCopy(transform.invT());

    for (int i = 0; i < 500; i++) {

        Point3f p = r_loc.origin + r_loc.direction * t;
        float dist = sceneSDF(p);

        if (dist < 0.001) {
            t-=stepSize;
            for (int i = 0; i < 10; i++) {

                p = r_loc.origin + r_loc.direction * t;
                dist = sceneSDF(p);
                if (dist < 0.001) {
                    t -= smallStepSize;
                    p = r_loc.origin + r_loc.direction * t;
                    InitializeIntersection(isect, t, p);
                    return true;
                }
                t+= smallStepSize;
            }
            return false;
        }
        t+= stepSize;
    }

    return false;
}

Vector3f estimateNormal(Vector3f pos) {

    float epsilon = 0.0001f;
    Vector3f normal = Vector3f(0.f);

    normal.x = sceneSDF(Point3f(pos.x + epsilon, pos.y, pos.z))
              - sceneSDF(Point3f(pos.x - epsilon, pos.y, pos.z));
    normal.y = sceneSDF(Point3f(pos.x, pos.y + epsilon, pos.z))
              - sceneSDF(Point3f(pos.x, pos.y - epsilon, pos.z));
    normal.z = sceneSDF(Point3f(pos.x, pos.y, pos.z + epsilon))
              - sceneSDF(Point3f(pos.x, pos.y, pos.z - epsilon));

    return glm::normalize(normal);
}

void Implicit::ComputeTBN(const Point3f& P, Normal3f* nor, Vector3f* tan, Vector3f* bit) const
{
    *nor = glm::normalize(transform.invTransT() * estimateNormal(P));

    *tan = glm::normalize(transform.T3() * glm::cross(Vector3f(0,1,0),estimateNormal(P)));
    *bit = glm::normalize(glm::cross(*nor, *tan));
}

Intersection Implicit::Sample(const Point2f &xi, Float *pdf) const
{
    return Intersection();
}
