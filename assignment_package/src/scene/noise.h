#pragma once

#include "globals.h"
#include "math.h"

class Noise
{
public:
    Noise()
    {}

    float getNoise(Vector3f p);
    float Noise3D(Vector3f p);
    float CosineInterpolate(float a, float b, float t);
    float SmoothedNoise(Vector3f p);
    float Noisehash(Vector3f p);
    Vector3f fract(Vector3f p);
    float fract(float t);
};
