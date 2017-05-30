#include "noise.h"

float Noise::getNoise(Vector3f p)
{
    //multi-octave noise
    float total = 0.0f;
    float persistence = 0.8f;
    int numOctaves = 5;

    //Loop over n octaves
    for(int i=0; i<numOctaves; i++)
    {
        float frequency = std::pow(2, i);
        float amplitude = std::pow(persistence, i);

        //sum up all the octaves
        total += Noise3D(p * frequency) * (1.0f/amplitude);
    }
    return total;
}

float Noise::Noise3D(Vector3f p)
{
    Vector3f p1 = Vector3f( std::floor(p.x), std::floor(p.y), std::floor(p.z) );
    Vector3f p2 = Vector3f( std::floor(p.x), std::floor(p.y), std::ceil(p.z)  );
    Vector3f p3 = Vector3f( std::floor(p.x), std::ceil(p.y),  std::floor(p.z) );
    Vector3f p4 = Vector3f( std::floor(p.x), std::ceil(p.y),  std::ceil(p.z)  );
    Vector3f p5 = Vector3f( std::ceil(p.x),  std::floor(p.y), std::floor(p.z) );
    Vector3f p6 = Vector3f( std::ceil(p.x),  std::floor(p.y), std::ceil(p.z)  );
    Vector3f p7 = Vector3f( std::ceil(p.x),  std::ceil(p.y),  std::floor(p.z) );
    Vector3f p8 = Vector3f( std::ceil(p.x),  std::ceil(p.y),  std::ceil(p.z)  );

    float v1 = SmoothedNoise(p1);
    float v2 = SmoothedNoise(p2);
    float v3 = SmoothedNoise(p3);
    float v4 = SmoothedNoise(p4);
    float v5 = SmoothedNoise(p5);
    float v6 = SmoothedNoise(p6);
    float v7 = SmoothedNoise(p7);
    float v8 = SmoothedNoise(p8);

    float i1 = CosineInterpolate(v1 , v2 , p.z-std::floor(p.z));
    float i2 = CosineInterpolate(v3 , v4 , p.z-std::floor(p.z));
    float i3 = CosineInterpolate(v5 , v6 , p.z-std::floor(p.z));
    float i4 = CosineInterpolate(v7 , v8 , p.z-std::floor(p.z));

    float i5 = CosineInterpolate(i1 , i2 , p.y-std::floor(p.y));
    float i6 = CosineInterpolate(i3 , i4 , p.y-std::floor(p.y));

    float interpolatedNoise = CosineInterpolate(i5 , i6 , p.x-std::floor(p.x));

    return interpolatedNoise;
}

float Noise::CosineInterpolate(float a, float b, float t)
{
    // a --- the lower bound value of interpolation
    // b --- the upper bound value of interpolation

    float ft = t * 3.1415927f;
    float f = (1.0f - cos(ft)) * 0.5f;

    return  a*(1.0f-f) + b*f;
}

float Noise::SmoothedNoise(Vector3f p)
{
    Vector3f p1 = Vector3f( p.x - 1.0f, p.y + 1.0f, p.z + 1.0f );
    Vector3f p2 = Vector3f( p.x,        p.y + 1.0f, p.z + 1.0f );
    Vector3f p3 = Vector3f( p.x + 1.0f, p.y + 1.0f, p.z + 1.0f );
    Vector3f p4 = Vector3f( p.x - 1.0f, p.y,        p.z + 1.0f );
    Vector3f p5 = Vector3f( p.x,        p.y,        p.z + 1.0f );
    Vector3f p6 = Vector3f( p.x + 1.0f, p.y,        p.z + 1.0f );
    Vector3f p7 = Vector3f( p.x - 1.0f, p.y - 1.0f, p.z + 1.0f );
    Vector3f p8 = Vector3f( p.x,        p.y - 1.0f, p.z + 1.0f );
    Vector3f p9 = Vector3f( p.x + 1.0f, p.y - 1.0f, p.z + 1.0f );

    Vector3f p10 = Vector3f( p.x - 1.0f, p.y + 1.0f, p.z );
    Vector3f p11 = Vector3f( p.x,        p.y + 1.0f, p.z );
    Vector3f p12 = Vector3f( p.x + 1.0f, p.y + 1.0f, p.z );
    Vector3f p13 = Vector3f( p.x - 1.0f, p.y,        p.z );
    Vector3f p14 = Vector3f( p.x + 1.0f, p.y,        p.z );
    Vector3f p15 = Vector3f( p.x - 1.0f, p.y - 1.0f, p.z );
    Vector3f p16 = Vector3f( p.x,        p.y - 1.0f, p.z );
    Vector3f p17 = Vector3f( p.x + 1.0f, p.y - 1.0f, p.z );

    Vector3f p18 = Vector3f( p.x - 1.0f, p.y + 1.0f, p.z - 1.0f );
    Vector3f p19 = Vector3f( p.x,        p.y + 1.0f, p.z - 1.0f );
    Vector3f p20 = Vector3f( p.x + 1.0f, p.y + 1.0f, p.z - 1.0f );
    Vector3f p21 = Vector3f( p.x - 1.0f, p.y,        p.z - 1.0f );
    Vector3f p22 = Vector3f( p.x,        p.y,        p.z - 1.0f );
    Vector3f p23 = Vector3f( p.x + 1.0f, p.y,        p.z - 1.0f );
    Vector3f p24 = Vector3f( p.x - 1.0f, p.y - 1.0f, p.z - 1.0f );
    Vector3f p25 = Vector3f( p.x,        p.y - 1.0f, p.z - 1.0f );
    Vector3f p26 = Vector3f( p.x + 1.0f, p.y - 1.0f, p.z - 1.0f );

    float influence1 = 4.0/100.0;
    float influence2 = 1.8/100.0;
    float influence3 = 40.0/100.0;
    //make sure 6*influnce1 + 20*influence2=1

    float n1 =  influence2 * Noisehash( p1 );
    float n2 =  influence2 * Noisehash( p2 );
    float n3 =  influence2 * Noisehash( p3 );
    float n4 =  influence2 * Noisehash( p4 );
    float n5 =  influence1 * Noisehash( p5 );
    float n6 =  influence2 * Noisehash( p6 );
    float n7 =  influence2 * Noisehash( p7 );
    float n8 =  influence2 * Noisehash( p8 );
    float n9 =  influence2 * Noisehash( p9 );

    float n10 = influence2 * Noisehash( p10 );
    float n11 = influence1 * Noisehash( p11 );
    float n12 = influence2 * Noisehash( p12 );
    float n13 = influence1 * Noisehash( p13 );
    float n14 = influence3 * Noisehash( p13 );
    float n15 = influence1 * Noisehash( p14 );
    float n16 = influence2 * Noisehash( p15 );
    float n17 = influence1 * Noisehash( p16 );
    float n18 = influence2 * Noisehash( p17 );

    float n19 = influence2 * Noisehash( p18 );
    float n20 = influence2 * Noisehash( p19 );
    float n21 = influence2 * Noisehash( p20 );
    float n22 = influence2 * Noisehash( p21 );
    float n23 = influence1 * Noisehash( p22 );
    float n24 = influence2 * Noisehash( p23 );
    float n25 = influence2 * Noisehash( p24 );
    float n26 = influence2 * Noisehash( p25 );
    float n27 = influence2 * Noisehash( p26 );

    float average = n1 + n2 +n3 + n4 + n5 + n6 +n7 + n8 + n9 + n10 +n11 + n12 + n13 +
                    n14 + n15 + n16 + n17 + n18 +n19 + n20 + n21 + n22 +n23 + n24 + n25 + n26 +n27;

    return average;
}

float Noise::Noisehash(Vector3f p)
{
    p  = fract( p*Vector3f(0.3183099) + Vector3f(0.1) );
    p *= 17.0;
    return fract( p.x*p.y*p.z*(p.x+p.y+p.z) );
}

Vector3f Noise::fract(Vector3f p)
{
    p.x = p.x - std::floor(p.x);
    p.y = p.y - std::floor(p.y);
    p.z = p.z - std::floor(p.z);
}

float Noise::fract(float t)
{
    t = t - std::floor(t);
}
