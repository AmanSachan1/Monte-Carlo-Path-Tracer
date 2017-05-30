#include "thinlenscamera.h"

ThinLensCamera::ThinLensCamera(): ThinLensCamera(2.0, 3.0)
{}

ThinLensCamera::ThinLensCamera( float lensr, float focald ):
    Camera(), lensRadius(lensr), focalDistance(focald)
{}

//ThinLensCamera::ThinLensCamera( unsigned int w, unsigned int h, float lensr, float focald ):
//    Camera(w, h), ThinLensCamera(lensr, focald)
//{}

//ThinLensCamera::ThinLensCamera( unsigned int w, unsigned int h, const Vector3f &e, const Vector3f &r, const Vector3f &worldUp, float lensr, float focald ):
//    Camera(w, h, e, r, worldUp), ThinLensCamera(lensr, focald)
//{}

//ThinLensCamera::ThinLensCamera( const Camera &c, float lensr, float focald ):
//    Camera(c), ThinLensCamera(lensr, focald)
//{}
