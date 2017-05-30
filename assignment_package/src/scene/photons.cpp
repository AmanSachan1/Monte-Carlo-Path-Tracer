#include "photons.h"

KdTree* Photons::createTree(KdTree* root)
{
    shootPhotons();
    constructTree(root);
}

void Photons::shootPhotons()
{
    int numLights = scene->lights.size();
    int n1 = numPhotons/numLights;

    if(n1<1000)
    {
        n1 = 1000;
    }

    int n2 = (int)std::ceil(n1/100);

    for(int i=0; i<numLights; i++)
    {
        for(int j=0; i<100; i++) //done to make it easy to convert to a parallel structure
        {
            for(int k=0; i<n2; i++)
            {
                shootPhotonsHelper(*scene, sampler, recursionLimit);
            }
        }
    }
}

void Photons::shootPhotonsHelper(const Scene& scene, std::shared_ptr<Sampler> sampler, int depth)
{

}

KdTree* Photons::constructTree(KdTree* root)
{

}
