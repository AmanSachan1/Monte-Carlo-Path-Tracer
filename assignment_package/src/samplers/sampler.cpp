#include "sampler.h"

Sampler::Sampler(int samplesPerPixel, int seed)
    : samplesPerPixel(samplesPerPixel), rng(seed)
{}

float Sampler::Get1D()
{
    return rng.nextFloat();
}

Point2f Sampler::Get2D()
{
    return Point2f(rng.nextFloat(), rng.nextFloat());
}

std::unique_ptr<Sampler> Sampler::Clone(int seed)
{
    Sampler* s = new Sampler(samplesPerPixel, seed);
    return std::unique_ptr<Sampler>(s);
}

std::vector<Point2f> Sampler::GenerateStratifiedSamples()
{
    std::vector<Point2f> samples;
    // The square root of the number of samples input
    int sqrtVal = (int) (std::sqrt((float) samplesPerPixel) + 0.5);
    // A number useful for scaling a square of size sqrtVal x sqrtVal to 1 x 1
    float invSqrtVal = 1.f / sqrtVal;

    samplesPerPixel = sqrtVal * sqrtVal;
    samples.resize(samplesPerPixel);

    for(int i = 0; i < samplesPerPixel; ++i)
    {
        int y = i / sqrtVal;
        int x = i % sqrtVal;
        glm::vec2 sample;

        sample = glm::vec2((x + rng.nextFloat()) * invSqrtVal,
                           (y + rng.nextFloat()) * invSqrtVal);
        samples[i] = sample;
    }
    return samples;
}
