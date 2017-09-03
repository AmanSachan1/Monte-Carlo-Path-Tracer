#pragma once
#include <QList>
#include <raytracing/film.h>
#include <scene/camera.h>
#include <scene/lights/light.h>
#include <scene/geometry/shape.h>
#include <scene/medium.h>
#include <scene/photons.h>
#include "bvh.h"
#include "scene/kdtree.h"

class Primitive;
class BVHAccel;
class Material;
class Light;
class KdTree;
class Photons;

class Scene
{
public:
    Scene();
    ~Scene();
    QList<std::shared_ptr<Primitive>> primitives;
    QList<std::shared_ptr<Material>> materials;
    QList<std::shared_ptr<Light>> lights;
    Camera camera;
    std::shared_ptr<Medium> medium;
    Bounds3f bounds;

    bool hasMedium;

    Film film;

    BVHAccel* bvh;
    KdTree* kdTree;
    Photons* photons;

    QList<std::shared_ptr<Drawable>> drawables;

    void SetCamera(const Camera &c);

    void CreateTestScene();
    void Clear();

    bool Intersect(const Ray& ray, Intersection* isect) const;

    void clearBVH();
    void clearkdTree();
};
