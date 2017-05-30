#pragma once
#include "camera.h"

#include <globals.h>
#include <la.h>
#include <iostream>

class ThinLensCamera: public Camera
{
private:
    float lensRadius;
    float focalDistance;
public:
    ThinLensCamera();
    ThinLensCamera( float lensr, float focald );
    ThinLensCamera( unsigned int w, unsigned int h, float lensr, float focald );
    ThinLensCamera( unsigned int w, unsigned int h, const Vector3f &e, const Vector3f &r, const Vector3f &worldUp, float lensr, float focald );
    ThinLensCamera( const Camera &c, float lensr, float focald );
};
