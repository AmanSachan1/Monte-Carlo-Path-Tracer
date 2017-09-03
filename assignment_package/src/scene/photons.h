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
class KdTree;
struct KdNode;

class Photons: public QRunnable
{
public:
    Photons(Scene* s, std::shared_ptr<Sampler> sampler, int recursionLimit)
        : scene(s), sampler(sampler), recursionLimit(recursionLimit)
    {}

    virtual ~Photons() {}

    virtual void run(); // A function declared in the QRunnable class that we must implement
                        // in order for our class to be compatible with a QThreadPool.
    virtual void shoot(); //Called from run() for multi threading

    KdNode *createTree(KdTree*& kdTree);
    void shootPhotons();
    void shootPhotonsHelper(const Scene& scene, std::shared_ptr<Sampler> sampler,
                            int depth, Ray& ray,  int &lightIndex, float alpha,
                            std::vector<Photon *>* threadPhotonsDirect,
                            std::vector<Photon *>* threadPhotonsIndirect,
                            std::vector<Photon *>* threadPhotonsCaustic);
    bool RussianRoulette(Color3f& energy, float probability, int depth) const;
    KdTree* constructTree(KdTree* root);

protected:
    Scene const * const scene;              // A pointer to the Scene instance stored in MyGL.
    std::shared_ptr<Sampler> sampler;       // A pointer to the Sampler that we will use to generate pixel samples for this thread.

    int recursionLimit;
    std::vector<std::vector<Photon*>*> photonListDirect;
    std::vector<std::vector<Photon*>*> photonListIndirect;
    std::vector<std::vector<Photon*>*> photonListCaustic;
    int numPhotons=0;
};
