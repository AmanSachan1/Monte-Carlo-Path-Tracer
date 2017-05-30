#pragma once
#include <QRunnable>
#include <raytracing/film.h>
#include <scene/camera.h>
#include <scene/scene.h>
#include <globals.h>
#include <samplers/sampler.h>
#include <scene/lights/light.h>
#include "scene/kdtree.h"

struct Photon
{
    Photon() {}
    Photon(Vector3f p, Vector3f incidentDirection, float power, Color3f c)
        : position(p), wi(incidentDirection), alpha(power), color(c)
    {}

    Vector3f position;
    Vector3f wi; //incident direction
    float alpha; //photon power
    Color3f color;
};

class Scene;

class Photons
{
public:
    Photons(Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit)
        : scene(s), sampler(sampler), recursionLimit(recursionLimit)
    {}

    ~Photons() {}

    KdTree* createTree(KdTree* root);
    void shootPhotons();
    void shootPhotonsHelper(const Scene& scene, std::shared_ptr<Sampler> sampler, int depth);
    KdTree* constructTree(KdTree* root);

protected:
    Scene const * const scene;              // A pointer to the Scene instance stored in MyGL.
    std::shared_ptr<Sampler> sampler;       // A pointer to the Sampler that we will use to generate pixel samples for this thread.

    int recursionLimit;
    std::vector<Photon> photonList;
    int numPhotons=0;
};
