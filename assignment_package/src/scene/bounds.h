#pragma once
#include <globals.h>
#include "transform.h"
#include <raytracing/ray.h>


class Bounds3f
{
public:
    Point3f min, max;

    Bounds3f()
        : min(std::numeric_limits<Float>::infinity()), max(-std::numeric_limits<Float>::infinity())
    {}
    Bounds3f(const Point3f& min, const Point3f& max)
        : min(min), max(max)
    {}
    Bounds3f(const Point3f& p)
        : min(p), max(p)
    {}

    // Returns a vector representing the diagonal of the box
    Vector3f Diagonal() const { return max - min; }

    // Returns the index of the axis with the largest length
    int MaximumExtent() const
    {
        Vector3f d = Diagonal();
        if (d.x > d.y && d.x > d.z)
            return 0;
        else if (d.y > d.z)
            return 1;
        else
            return 2;
    }

    // Returns the position of a point *relative*
    // to the min and max corners of the box.
    // This ranges from (0, 0, 0) to (1, 1, 1)
    // where these two extremes represent the
    // values the result would have at the min and
    // max corners of the box
    Vector3f Offset(const Point3f &p) const
    {
        Vector3f o = p - min;
        if (max.x > min.x) o.x /= max.x - min.x;
        if (max.y > min.y) o.y /= max.y - min.y;
        if (max.z > min.z) o.z /= max.z - min.z;
        return o;
    }

    // Transforms this Bounds3f by the input Transform and
    // also returns a Bounds3f representing our new boundaries.
    // Transforming a Bounds3f is equivalent to creating a Bounds3f
    // that encompasses the non-axis-aligned box resulting from
    // transforming this Bounds3f's eight corners
    Bounds3f Apply(const Transform& tr);

    // Returns the surface area of this bounding box
    float SurfaceArea() const;

    bool Intersect(const Ray& r , float* t) const;

    /*
    //DELETEME
    inline const Point3f& operator[](int i) const {
        return (i == 0) ? min : max;
    }

    //DELETEME
    inline Point3f operator[](int i) {
        return (i == 0) ? min : max;
    }

    //DELETEME
    static inline Float gamma(int n) {
        return (n *     std::numeric_limits<Float>::epsilon() * 0.5
                ) / (1 - n *     std::numeric_limits<Float>::epsilon() * 0.5
                     );
    }

    //DELETEME
    inline bool IntersectP(const Ray &ray, const Vector3f &invDir,
                           const int dirIsNeg[3]) const {
        const Bounds3f &bounds = *this;
        // Check for ray intersection against $x$ and $y$ slabs
        Float tMin = (bounds[dirIsNeg[0]].x - ray.origin.x) * invDir.x;
        Float tMax = (bounds[1 - dirIsNeg[0]].x - ray.origin.x) * invDir.x;
        Float tyMin = (bounds[dirIsNeg[1]].y - ray.origin.y) * invDir.y;
        Float tyMax = (bounds[1 - dirIsNeg[1]].y - ray.origin.y) * invDir.y;

        // Update _tMax_ and _tyMax_ to ensure robust bounds intersection
        tMax *= 1 + 2 * gamma(3);
        tyMax *= 1 + 2 * gamma(3);
        if (tMin > tyMax || tyMin > tMax) return false;
        if (tyMin > tMin) tMin = tyMin;
        if (tyMax < tMax) tMax = tyMax;

        // Check for ray intersection against $z$ slab
        Float tzMin = (bounds[dirIsNeg[2]].z - ray.origin.z) * invDir.z;
        Float tzMax = (bounds[1 - dirIsNeg[2]].z - ray.origin.z) * invDir.z;

        // Update _tzMax_ to ensure robust bounds intersection
        tzMax *= 1 + 2 * gamma(3);
        if (tMin > tzMax || tzMin > tMax) return false;
        if (tzMin > tMin) tMin = tzMin;
        if (tzMax < tMax) tMax = tzMax;
        return (tMax > 0);
    }
    */    
};

Bounds3f Union(const Bounds3f& b1, const Bounds3f& b2);
Bounds3f Union(const Bounds3f& b1, const Point3f& p);
Bounds3f Union(const Bounds3f& b1, const glm::vec4& p);
