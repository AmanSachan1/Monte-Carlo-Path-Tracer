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

    bool Contains(Vector3f& pos) const
    {
        if((pos.x>min.x && pos.y>min.y && pos.z>min.z) &&
           (pos.x<max.x && pos.y<max.y && pos.z<max.z))
        {
            return true;
        }

        return false;
    }
};

Bounds3f Union(const Bounds3f& b1, const Bounds3f& b2);
Bounds3f Union(const Bounds3f& b1, const Point3f& p);
Bounds3f Union(const Bounds3f& b1, const glm::vec4& p);
