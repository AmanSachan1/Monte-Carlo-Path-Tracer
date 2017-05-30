#include "shape.h"
#include <QDateTime>

pcg32 Shape::colorRNG = pcg32(QDateTime::currentMSecsSinceEpoch());

void Shape::InitializeIntersection(Intersection *isect, float t, Point3f pLocal) const
{
    isect->point = Point3f(transform.T() * glm::vec4(pLocal, 1));
    ComputeTBN(pLocal, &(isect->normalGeometric), &(isect->tangent), &(isect->bitangent));
    isect->uv = GetUVCoordinates(pLocal);
    isect->t = t;
}

Intersection Shape::Sample(const Intersection &ref, const Point2f &xi, float *pdf) const
{
    Intersection isect = Sample(xi, pdf);

    Vector3f wi = glm::normalize(isect.point - ref.point);

    float absDot = AbsDot(isect.normalGeometric, -wi);

    if(absDot == 0.0f)
    {
        *pdf = 0.0f;
    }
    else
    {
        *pdf = glm::distance2(isect.point, ref.point)/(absDot*Area());
    }

    return isect;
}
