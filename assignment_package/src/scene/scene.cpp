#include <scene/scene.h>

#include <scene/geometry/cube.h>
#include <scene/geometry/sphere.h>
#include <scene/geometry/mesh.h>
#include <scene/geometry/squareplane.h>
#include <scene/materials/mattematerial.h>
#include <scene/lights/diffusearealight.h>


Scene::Scene() : bvh(nullptr), kdTree(nullptr), medium(nullptr), bounds()
{}

Scene::~Scene()
{
    delete bvh;
    delete kdTree;
}

void Scene::SetCamera(const Camera &c)
{
    camera = Camera(c);
    camera.create();
    film.SetDimensions(c.width, c.height);
}

bool Scene::Intersect(const Ray &ray, Intersection *isect) const
{
    if(bvh)
    {
        return bvh->Intersect(ray, isect);
    }
    else
    {
        bool result = false;
        for(std::shared_ptr<Primitive> p : primitives)
        {
            Intersection testIsect;
            if(p->Intersect(ray, &testIsect))
            {
                if(testIsect.t < isect->t || isect->t < 0)
                {
                    *isect = testIsect;
                    result = true;
                }
            }
        }
        return result;
    }
    return false;
}

void Scene::CreateTestScene()
{
    //Floor
    //Area light
    //Figure in front of light

    auto matteWhite = std::make_shared<MatteMaterial>(Color3f(1,1,1), 0, nullptr, nullptr);
    auto matteRed = std::make_shared<MatteMaterial>(Color3f(1,0,0), 0, nullptr, nullptr);
    auto matteGreen = std::make_shared<MatteMaterial>(Color3f(0,1,0), 0, nullptr, nullptr);

    // Floor, which is a large white plane
    auto floor = std::make_shared<SquarePlane>();
    floor->transform = Transform(Vector3f(0,0,0), Vector3f(-90,0,0), Vector3f(10,10,1));
    auto floorPrim = std::make_shared<Primitive>(floor);
    floorPrim->material = matteWhite;
    floorPrim->name = QString("Floor");

    // Light source, which is a diffuse area light with a large plane as its shape
    auto lightSquare = std::make_shared<SquarePlane>();
    lightSquare->transform = Transform(Vector3f(0,2.5f,5), Vector3f(0,180,0), Vector3f(8, 5, 1));
    auto lightSource = std::make_shared<DiffuseAreaLight>(lightSquare->transform, Color3f(1,1,1) * 2.f, lightSquare);
    auto lightPrim = std::make_shared<Primitive>(lightSquare, nullptr, lightSource);
    lightPrim->name = QString("Light Source");
    lightSource->name = QString("Light Source 1");

    // Light source 2, which is a diffuse area light with a large plane as its shape
    auto lightSquare2 = std::make_shared<SquarePlane>();
    lightSquare2->transform = Transform(Vector3f(5,2.5f,0), Vector3f(0,90,0), Vector3f(8, 5, 1));
    auto lightSource2 = std::make_shared<DiffuseAreaLight>(lightSquare2->transform, Color3f(0.9,1,0.7) * 2.f, lightSquare2, true);
    auto lightPrim2 = std::make_shared<Primitive>(lightSquare2, nullptr, lightSource2);
    lightPrim2->name = QString("Light Source 2");
    lightSource2->name = QString("Light Source 2");

    // Shadow casting shape, which is a red sphere
    auto sphere = std::make_shared<Sphere>();
    sphere->transform = Transform(Vector3f(0,1,0), Vector3f(0,0,0), Vector3f(1,1,1));
    auto spherePrim = std::make_shared<Primitive>(sphere);
    spherePrim->material = matteRed;
    spherePrim->name = QString("Red Sphere");

    // Back wall, which is a green rectangle
    auto greenWall = std::make_shared<SquarePlane>();
    greenWall->transform = Transform(Vector3f(-5,2.5f,0), Vector3f(0,90,0), Vector3f(10, 5, 1));
    auto greenWallPrim = std::make_shared<Primitive>(greenWall);
    greenWallPrim->material = matteGreen;
    greenWallPrim->name = QString("Wall");


    primitives.append(floorPrim);
    primitives.append(lightPrim);
    primitives.append(lightPrim2);
    primitives.append(spherePrim);
    primitives.append(greenWallPrim);

    lights.append(lightSource);
    lights.append(lightSource2);

    for(std::shared_ptr<Primitive> p : primitives)
    {
        p->shape->create();
    }

    camera = Camera(400, 400, Point3f(5, 8, -5), Point3f(0,0,0), Vector3f(0,1,0), 0.0f, 0.0f);
    camera.near_clip = 0.1f;
    camera.far_clip = 100.0f;
    camera.create();
    film = Film(400, 400);
}

void Scene::Clear()
{
    // These lists contain shared_ptrs
    // so the pointers will be freed
    // if appropriate when we clear the lists.
    primitives.clear();
    lights.clear();
    materials.clear();
    drawables.clear();
    camera = Camera();
    film = Film();
}

void Scene::clearBVH()
{
    if(bvh)
    {
        delete bvh;
        bvh = nullptr;
    }
}

void Scene::clearkdTree()
{
    if(kdTree)
    {
        delete kdTree;
        kdTree = nullptr;
    }
}
