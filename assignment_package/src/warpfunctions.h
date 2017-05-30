#pragma once
#include "globals.h"

//a collection of functions for sampling various shapes

class WarpFunctions
{
public:
    // Uniformly sample a vector on a 2D disk with radius 1, centered around the origin
    static Point3f squareToDiskUniform(const Point2f &sample);

    // Concentrically sample a vector on a 2D disk with radius 1, centered around the origin
    static Point3f squareToDiskConcentric(const Point2f &sample);

    static float squareToDiskPDF(const Point3f &sample);

    // Uniformly sample a vector on the unit sphere with respect to solid angles
    static Point3f squareToSphereUniform(const Point2f &sample);

    static float squareToSphereUniformPDF(const Point3f &sample);

    /**
     * \brief Uniformly sample a vector on a spherical cap around (0, 0, 1)
     *
     * A spherical cap is the subset of a unit sphere whose directions
     * make an angle of less than 'theta' with the north pole. This function
     * expects the cosine of 'theta' as a parameter.
     */
    static Point3f squareToSphereCapUniform(const Point2f &sample, float thetaMin);

    static float squareToSphereCapUniformPDF(const Point3f &sample, float thetaMin);

    // Uniformly sample a vector on the unit hemisphere around the pole (0,0,1) with respect to solid angles
    static Point3f squareToHemisphereUniform(const Point2f &sample);

    static float squareToHemisphereUniformPDF(const Point3f &sample);

    // Uniformly sample a vector on the unit hemisphere around the pole (0,0,1) with respect to projected solid angles
    static Point3f squareToHemisphereCosine(const Point2f &sample);

    static float squareToHemisphereCosinePDF(const Point3f &sample);
};
