#pragma once
#include <globals.h>
#include <raytracing/intersection.h>
#include <QImage>
#include <QColor>


// A Material is an interface class designed to produce a
// temporary BSDF for a given Intersection with the Primitive
// to which it is attached.
// Depending on the type of Material producing the BSDF, it
// may produce several BxDFs attached to the BSDF. For example,
// a GlassMaterial would produce a BSDF containing a specular
// reflection BRDF and a specular transmission BTDF.
class Material {
  public:
    virtual void ProduceBSDF(Intersection *isect) const = 0;
    virtual ~Material(){}

    static Color3f GetImageColor(const Point2f &uv_coord, const QImage* const image)
    {
        if(image)
        {
            int X = glm::min(image->width() * uv_coord.x, image->width() - 1.0f);
            int Y = glm::min(image->height() * (1.0f - uv_coord.y), image->height() - 1.0f);
            QColor color = image->pixel(X, Y);
            return Color3f(color.red(), color.green(), color.blue())/255.0f;
        }
        return Color3f(1.f, 1.f, 1.f);
    }

    // You'll notice that the base Material class contains no
    // maps of any sort. The non-abstract subclasses of Material
    // provide various maps depending on their type, so that
    // one can map all sorts of attributes to textures, such as
    // reflectivity, albedo, roughness, and surface normal direction.
};
