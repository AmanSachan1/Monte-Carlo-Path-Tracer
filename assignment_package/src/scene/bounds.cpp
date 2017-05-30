#include "bounds.h"

bool Bounds3f::Intersect(const Ray &r, float* t) const
{
    float t_n = -1000000;
    float t_f = 1000000;
    for(int i = 0; i < 3; i++)
    {
        //Ray parallel to slab check
        if(r.direction[i] == 0)
        {
            if(r.origin[i] < min[i] || r.origin[i] > max[i])
            {
                //The ray is parallel to the slab
                return false;
            }
        }
        //If not parallel, do slab intersect check
        float t0 = (min[i] - r.origin[i])/r.direction[i];
        float t1 = (max[i] - r.origin[i])/r.direction[i];
        if(t0 > t1)
        {
            float temp = t1;
            t1 = t0;
            t0 = temp;
        }
        if(t0 > t_n)
        {
            t_n = t0;
        }
        if(t1 < t_f)
        {
            t_f = t1;
        }
    }
    //making sure ray intersected the correct slab
    bool flag_in_box = false;

    if( max[0]-r.origin.x > min[0] &&
        max[1]-r.origin.y > min[1] &&
        max[2]-r.origin.z > min[2] )
    {
        flag_in_box = true;
    }

    if(t_n < t_f)
    {
        float t = t_n > 0 ? t_n : t_f; //ensure ray moves in the positive direction
        if(t < 0 && !flag_in_box)
        {
            return false;
        }

        return true;
    }
    else //If t_near was greater than t_far, we did not hit the cube
    {
        return false;
    }

    return false;
}

Bounds3f Bounds3f::Apply(const Transform &tr)
{
    //transform the min and max points of the bounding
    //box by the transformation tr
    Bounds3f b = Bounds3f();

    glm::vec3 c1 = glm::vec3( min.x, min.y, min.z );
    glm::vec3 c2 = glm::vec3( min.x, min.y, max.z );
    glm::vec3 c3 = glm::vec3( min.x, max.y, min.z );
    glm::vec3 c4 = glm::vec3( min.x, max.y, max.z );
    glm::vec3 c5 = glm::vec3( max.x, min.y, min.z );
    glm::vec3 c6 = glm::vec3( max.x, min.y, max.z );
    glm::vec3 c7 = glm::vec3( max.x, max.y, min.z );
    glm::vec3 c8 = glm::vec3( max.x, max.y, max.z );

    c1 = glm::vec3( tr.T()*glm::vec4(c1, 1.0f) );
    c2 = glm::vec3( tr.T()*glm::vec4(c2, 1.0f) );
    c3 = glm::vec3( tr.T()*glm::vec4(c3, 1.0f) );
    c4 = glm::vec3( tr.T()*glm::vec4(c4, 1.0f) );
    c5 = glm::vec3( tr.T()*glm::vec4(c5, 1.0f) );
    c6 = glm::vec3( tr.T()*glm::vec4(c6, 1.0f) );
    c7 = glm::vec3( tr.T()*glm::vec4(c7, 1.0f) );
    c8 = glm::vec3( tr.T()*glm::vec4(c8, 1.0f) );

    b = Union(b, c1);
    b = Union(b, c2);
    b = Union(b, c3);
    b = Union(b, c4);
    b = Union(b, c5);
    b = Union(b, c6);
    b = Union(b, c7);
    b = Union(b, c8);

    this->min = b.min;
    this->max = b.max;

    return b;
}

float Bounds3f::SurfaceArea() const
{
    //use the surface area of the surrounding cuboid instead
    //of trying to find the surface area of the thing
    //actually inside
    float surfaceArea = 2*(max.x - min.x)*(max.y - min.y) +
                        2*(max.y - min.y)*(max.z - min.z) +
                        2*(max.z - min.z)*(max.x - min.x);

    return surfaceArea;
}

Bounds3f Union(const Bounds3f& b1, const Bounds3f& b2)
{
    return Bounds3f(Point3f(std::min(b1.min.x, b2.min.x),
                            std::min(b1.min.y, b2.min.y),
                            std::min(b1.min.z, b2.min.z)),
                    Point3f(std::max(b1.max.x, b2.max.x),
                            std::max(b1.max.y, b2.max.y),
                            std::max(b1.max.z, b2.max.z)));
}

Bounds3f Union(const Bounds3f& b1, const Point3f& p)
{
    return Bounds3f(Point3f(std::min(b1.min.x, p.x),
                            std::min(b1.min.y, p.y),
                            std::min(b1.min.z, p.z)),
                    Point3f(std::max(b1.max.x, p.x),
                            std::max(b1.max.y, p.y),
                            std::max(b1.max.z, p.z)));
}

Bounds3f Union(const Bounds3f& b1, const glm::vec4& p)
{
    return Union(b1, Point3f(p));
}
