#define _USE_MATH_DEFINES
#include "warpfunctions.h"
#include <math.h>

Point3f WarpFunctions::squareToDiskUniform(const Point2f &sample)
{
    float r = std::sqrt(sample[0]);
    float theta = 2* Pi * sample[1];
    float x = r*std::cos(theta);
    float y = r*std::sin(theta);
    float z=0.0f;
    return Point3f(x,y,z);
}

Point3f WarpFunctions::squareToDiskConcentric(const Point2f &sample)
{
    glm::vec2 sampleOffset = 2.0f*sample - glm::vec2(1,1);
    if(sampleOffset.x == 0 && sampleOffset.y ==0)
    {
        return Point3f(0.0f, 0.0f, 0.0f);
    }

    float theta, r;
    if(std::abs(sampleOffset.x) > std::abs(sampleOffset.y))
    {
        r = sampleOffset.x;
        theta = (Pi/4.0f) * (sampleOffset.y/sampleOffset.x);
    }
    else
    {
        r = sampleOffset.y;
        theta = (Pi/2.0f) - (Pi/4.0f) * (sampleOffset.x/sampleOffset.y);
    }
    return r*Point3f(std::cos(theta), std::sin(theta), 0.0f);
}

float WarpFunctions::squareToDiskPDF(const Point3f &sample)
{
    return InvPi;
}

Point3f WarpFunctions::squareToSphereUniform(const Point2f &sample)
{
    float z = 1-2*sample[0];
    float r = std::sqrt(std::max((float)0, (float)1.0 - z*z));
    float phi = 2*Pi*sample[1];
    return Point3f(r*std::cos(phi), r* std::sin(phi), z);
}

float WarpFunctions::squareToSphereUniformPDF(const Point3f &sample)
{
    return Inv4Pi;
}

Point3f WarpFunctions::squareToSphereCapUniform(const Point2f &sample, float thetaMin)
{
    float z = 1.0f - ((2 *(Pi-(thetaMin*Pi/180.f))/Pi)*sample[0]);
    float r = std::sqrt(std::max(0.0f, 1.0f - z*z));
    float phi = 2*Pi*sample[1];
    return Point3f(r*std::cos(phi), r* std::sin(phi), z);
}

float WarpFunctions::squareToSphereCapUniformPDF(const Point3f &sample, float thetaMin)
{
    float radian = (180.0f-thetaMin)*Pi/180.0f;
    return 1.0f/(2.0f*Pi*(1.0f-cos(radian)));
}

Point3f WarpFunctions::squareToHemisphereUniform(const Point2f &sample)
{
    float z = 1.0f - sample[0];
    float r = std::sqrt(std::max((float)0, (float)1.0 - z*z));
    float phi = 2*Pi*sample[1];
    return Point3f(r*std::cos(phi), r* std::sin(phi), z);
}

float WarpFunctions::squareToHemisphereUniformPDF(const Point3f &sample)
{
    return Inv2Pi;
}

Point3f WarpFunctions::squareToHemisphereCosine(const Point2f &sample)
{
    glm::vec3 d = squareToDiskConcentric(sample);
    float z = std::sqrt(std::max(0.0f, 1.0f - d.x*d.x - d.y*d.y));
    return Point3f(d.x, d.y, z);
}

float WarpFunctions::squareToHemisphereCosinePDF(const Point3f &sample)
{
    return std::cos(std::acos(sample[2]))*InvPi;
}
