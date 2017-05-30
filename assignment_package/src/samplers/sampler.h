#pragma once
#include <globals.h>
#include <pcg32.h>

// A class that generates 2D points via uniform random variables
class Sampler
{
public:
    // The seed determines which random numbers the RNG in
    // this sampler will produce. The seed will vary based on
    // the pixel tile to which this Sampler is assigned
    Sampler(int samplesPerPixel, int seed);

    float Get1D(); // Returns the result of calling our RNG's nextFloat() function.
    Point2f Get2D(); // Returns a 2D sample by calling our RNG's nextFloat() function.

    std::unique_ptr<Sampler> Clone(int seed);

    // A function to generate a collection of stratified samples in a square
    std::vector<Point2f> GenerateStratifiedSamples();

    int samplesPerPixel;

private:
    pcg32 rng;
};
