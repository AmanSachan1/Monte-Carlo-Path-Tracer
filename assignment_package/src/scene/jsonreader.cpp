#include <scene/jsonreader.h>
#include <scene/geometry/cube.h>
#include <scene/geometry/sphere.h>
#include <scene/geometry/mesh.h>
#include <scene/geometry/disc.h>
#include <scene/geometry/implicit.h>
#include <scene/geometry/squareplane.h>
#include <scene/materials/mattematerial.h>
#include <scene/materials/mirrormaterial.h>
#include <scene/materials/transmissivematerial.h>
#include <scene/materials/glassmaterial.h>
#include <scene/materials/plasticmaterial.h>
#include <scene/lights/diffusearealight.h>
#include <scene/lights/pointlights.h>
#include <scene/lights/spotlight.h>
#include <scene/lights/environmentlighting.h>

#include <scene/mediums/medium.h>
#include <scene/mediums/homogeneousmedia.h>
#include <scene/mediums/heterogeneousmedia.h>

#include <iostream>


void JSONReader::LoadSceneFromFile(QFile &file, const QStringRef &local_path, Scene &scene)
{
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray rawData = file.readAll();
        // Parse document
        QJsonDocument doc(QJsonDocument::fromJson(rawData));

        // Get JSON object
        QJsonObject json = doc.object();
        QJsonObject sceneObj, camera, _medium;
        QJsonArray primitiveList, materialList, lightList;

        QMap<QString, std::shared_ptr<Material>> mtl_name_to_material;
        QJsonArray frames = json["frames"].toArray();
        //check scene object for every frame
        foreach(const QJsonValue &frame, frames) {
            QJsonObject sceneObj = frame.toObject()["scene"].toObject();
            //load camera
            if(sceneObj.contains(QString("camera"))) {
                camera = sceneObj["camera"].toObject();
                scene.SetCamera(LoadCamera(camera));
            }

            //For Mediums -- volumetric rendering
            if(sceneObj.contains(QString("medium")))
            {
                _medium = sceneObj["medium"].toObject();
                scene.medium = LoadMedium(_medium);
            }

            //load all materials in QMap with mtl name as key and Material itself as value
            if(sceneObj.contains(QString("materials"))){
                materialList = sceneObj["materials"].toArray();
                foreach(const QJsonValue &materialVal, materialList){
                    QJsonObject materialObj = materialVal.toObject();
                    LoadMaterial(materialObj, local_path, &mtl_name_to_material);
                }
            }
            //load primitives and attach materials from QMap
            if(sceneObj.contains(QString("primitives"))) {
                primitiveList = sceneObj["primitives"].toArray();
                foreach(const QJsonValue &primitiveVal, primitiveList){
                    QJsonObject primitiveObj = primitiveVal.toObject();
                    LoadGeometry(primitiveObj, mtl_name_to_material, local_path, &scene.primitives, &scene.drawables);
                }
            }
            //load lights and attach materials from QMap
            if(sceneObj.contains(QString("lights"))) {
                lightList = sceneObj["lights"].toArray();
                foreach(const QJsonValue &lightVal, lightList){
                    QJsonObject lightObj = lightVal.toObject();
                    LoadLights(lightObj, mtl_name_to_material, local_path, &scene.primitives, &scene.lights, &scene.drawables);
                }
            }
        }

        for(std::shared_ptr<Drawable> d : scene.drawables)
        {
            d->create();
        }
        file.close();
    }
}

bool JSONReader::LoadGeometry(QJsonObject &geometry, QMap<QString, std::shared_ptr<Material>> mtl_map, const QStringRef &local_path, QList<std::shared_ptr<Primitive>> *primitives, QList<std::shared_ptr<Drawable>> *drawables)
{
    std::shared_ptr<Shape> shape = nullptr;
    //First check what type of geometry we're supposed to load
    QString type;
    if(geometry.contains(QString("shape"))){
        type = geometry["shape"].toString();
    }

    bool isMesh = false;
    if(QString::compare(type, QString("Mesh")) == 0)
    {
        auto mesh = std::make_shared<Mesh>();
        isMesh = true;

        Transform transform;
        if(geometry.contains(QString("transform"))) {
            QJsonObject qTransform = geometry["transform"].toObject();
            transform = LoadTransform(qTransform);
        }

        if(geometry.contains(QString("filename"))) {
            QString objFilePath = geometry["filename"].toString();
            std::static_pointer_cast<Mesh>(mesh)->LoadOBJ(QStringRef(&objFilePath), local_path, transform);
        }

        QString meshName("Unnamed Mesh");
        if(geometry.contains(QString("name"))) meshName = geometry["name"].toString();
        for(auto triangle : mesh->faces)
        {
            auto primitive = std::make_shared<Primitive>(triangle);
            QMap<QString, std::shared_ptr<Material>>::iterator i;
            if(geometry.contains(QString("material"))) {
                QString material_name = geometry["material"].toString();
                for (i = mtl_map.begin(); i != mtl_map.end(); ++i) {
                    if(i.key() == material_name){
                        primitive->material = i.value();
                    }
                }
            }
            primitive->name = meshName;
            (*primitives).append(primitive);
        }
        (*drawables).append(mesh);
    }
    else if(QString::compare(type, QString("Sphere")) == 0)
    {
        shape = std::make_shared<Sphere>();
    }
    else if(QString::compare(type, QString("SquarePlane")) == 0)
    {
        shape = std::make_shared<SquarePlane>();
    }
    else if(QString::compare(type, QString("Cube")) == 0)
    {
        shape = std::make_shared<Cube>();
    }
    else if(QString::compare(type, QString("Disc")) == 0)
    {
        shape = std::make_shared<Disc>();
    }
    else if(QString::compare(type, QString("Implicit")) == 0)
    {
        shape = std::make_shared<Implicit>();
    }
    else
    {
        std::cout << "Could not parse the geometry!" << std::endl;
        return NULL;
    }

    if(!isMesh)
    {
        // The Mesh class is handled differently
        // All Triangles are added to the Primitives list
        // but a single Drawable is created to render the Mesh
        auto primitive = std::make_shared<Primitive>(shape);
        QMap<QString, std::shared_ptr<Material>>::iterator i;
        if(geometry.contains(QString("material"))) {
            QString material_name = geometry["material"].toString();
            for (i = mtl_map.begin(); i != mtl_map.end(); ++i) {
                if(i.key() == material_name){
                    primitive->material = i.value();
                }
            }
        }
        //load transform to shape
        if(geometry.contains(QString("transform"))) {
            QJsonObject transform = geometry["transform"].toObject();
            shape->transform = LoadTransform(transform);
        }

//        //For Mediums -- volumetric rendering
//        if(geometry.contains(QString("medium")))
//        {
//            QJsonObject medium = geometry["medium"].toObject();
//            primitive->medium = LoadMedium(medium);
//            primitive->participatingMedia = true;
//        }

        if(geometry.contains(QString("name"))) primitive->name = geometry["name"].toString();
        (*primitives).append(primitive);
        (*drawables).append(primitive->shape);
    }

    return true;
}

bool JSONReader::LoadLights(QJsonObject &geometry, QMap<QString, std::shared_ptr<Material>> mtl_map, const QStringRef &local_path, QList<std::shared_ptr<Primitive>> *primitives, QList<std::shared_ptr<Light>> *lights, QList<std::shared_ptr<Drawable> > *drawables)
{
    std::shared_ptr<Shape> shape = nullptr;
    //First check what type of geometry we're supposed to load
    QString type;
    if(geometry.contains(QString("shape"))){
        type = geometry["shape"].toString();
    }

    if(QString::compare(type, QString("Mesh")) == 0)
    {
        Transform transform;
        auto mesh = std::make_shared<Mesh>();
        if(geometry.contains(QString("filename"))) {
            QString objFilePath = geometry["filename"].toString();
            std::static_pointer_cast<Mesh>(mesh)->LoadOBJ(QStringRef(&objFilePath), local_path, transform);
        }
    }
    else if(QString::compare(type, QString("Sphere")) == 0)
    {
        shape = std::make_shared<Sphere>();
    }
    else if(QString::compare(type, QString("SquarePlane")) == 0)
    {
        shape = std::make_shared<SquarePlane>();
    }
    else if(QString::compare(type, QString("Cube")) == 0)
    {
        shape = std::make_shared<Cube>();
    }
    else if(QString::compare(type, QString("Disc")) == 0)
    {
        shape = std::make_shared<Disc>();
    }
    else
    {
        std::cout << "Could not parse the geometry!" << std::endl;
        return NULL;
    }

    //load transform to shape
    if(geometry.contains(QString("transform"))) {
        QJsonObject transform = geometry["transform"].toObject();
        shape->transform = LoadTransform(transform);
    }

    //load light type
    std::shared_ptr<Light> lightType = nullptr;
    QString lgtType;
    if(geometry.contains(QString("type"))){
        lgtType = geometry["type"].toString();
    }

    if(QString::compare(lgtType, QString("DiffuseAreaLight")) == 0)
    {
        Color3f lightColor = ToVec3(geometry["lightColor"].toArray());
        Float intensity = static_cast< float >(geometry["intensity"].toDouble());
        bool twoSided = geometry.contains(QString("twoSided")) ? geometry["twoSided"].toBool() : false;
        lightType = std::make_shared<DiffuseAreaLight>(shape->transform, lightColor * intensity, shape, twoSided);
    }
    else if(QString::compare(lgtType, QString("PointLight")) == 0)
    {
        Color3f lightColor = ToVec3(geometry["lightColor"].toArray());
        Float intensity = static_cast< float >(geometry["intensity"].toDouble());
        lightType = std::make_shared<PointLight>(shape->transform, lightColor * intensity);
    }
    else if(QString::compare(lgtType, QString("SpotLight")) == 0)
    {
        Color3f lightColor = ToVec3(geometry["lightColor"].toArray());
        Float intensity = static_cast< float >(geometry["intensity"].toDouble());

        Float totalWidth = static_cast< float >(geometry["totalWidth"].toDouble());
        Float falloffStart = static_cast< float >(geometry["falloffStart"].toDouble());

        lightType = std::make_shared<Spotlight>(shape->transform, lightColor * intensity, totalWidth, falloffStart);
    }
    else if(QString::compare(lgtType, QString("EnvironmentLight")) == 0)
    {
        std::shared_ptr<QImage> texture;

        Float intensity = static_cast< float >(geometry["intensity"].toDouble());
        QString img_filepath = local_path.toString().append(geometry["texture"].toString());
        texture = std::make_shared<QImage>(img_filepath);

        lightType = std::make_shared<EnvironmentLight>(shape->transform, Color3f(0.f), texture, intensity);
    }
    else
    {
        std::cout << "Could not parse the geometry!" << std::endl;
        return NULL;
    }

    auto primitive = std::make_shared<Primitive>(shape, nullptr,  std::static_pointer_cast<AreaLight>(lightType));
    QMap<QString, std::shared_ptr<Material>>::iterator i;
    if(geometry.contains(QString("material"))) {
        QString material_name = geometry["material"].toString();
        for (i = mtl_map.begin(); i != mtl_map.end(); ++i) {
            if(i.key() == material_name){
                primitive->material = i.value();
            }
        }
    }

    if(geometry.contains(QString("name")))
    {
        primitive->name = geometry["name"].toString();
        lightType->name = geometry["name"].toString();
    }

    (*primitives).append(primitive);
    (*lights).append(lightType);
    (*drawables).append(primitive->shape);
    return true;
}

bool JSONReader::LoadMaterial(QJsonObject &material, const QStringRef &local_path, QMap<QString, std::shared_ptr<Material> > *mtl_map)
{
    QString type;

    //First check what type of material we're supposed to load
    if(material.contains(QString("type"))) type = material["type"].toString();

    if(QString::compare(type, QString("MatteMaterial")) == 0)
    {
        std::shared_ptr<QImage> textureMap;
        std::shared_ptr<QImage> normalMap;
        Color3f Kd = ToVec3(material["Kd"].toArray());
        Float sigma = static_cast< float >(material["sigma"].toDouble());
        if(material.contains(QString("textureMap"))) {
            QString img_filepath = local_path.toString().append(material["textureMap"].toString());
            textureMap = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("normalMap"))) {
            QString img_filepath = local_path.toString().append(material["normalMap"].toString());
            normalMap = std::make_shared<QImage>(img_filepath);
        }
        auto result = std::make_shared<MatteMaterial>(Kd, sigma, textureMap, normalMap);
        mtl_map->insert(material["name"].toString(), result);
    }
    else if(QString::compare(type, QString("MirrorMaterial")) == 0)
    {
        std::shared_ptr<QImage> roughnessMap;
        std::shared_ptr<QImage> textureMap;
        std::shared_ptr<QImage> normalMap;
        Color3f Kr = ToVec3(material["Kr"].toArray());
        float roughness = 0.f;
        if(material.contains(QString("roughness"))) {
            roughness = material["roughness"].toDouble();
        }
        if(material.contains(QString("roughnessMap"))) {
            QString img_filepath = local_path.toString().append(material["roughnessMap"].toString());
            roughnessMap = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("textureMap"))) {
            QString img_filepath = local_path.toString().append(material["textureMap"].toString());
            textureMap = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("normalMap"))) {
            QString img_filepath = local_path.toString().append(material["normalMap"].toString());
            normalMap = std::make_shared<QImage>(img_filepath);
        }
        auto result = std::make_shared<MirrorMaterial>(Kr, roughness, roughnessMap, textureMap, normalMap);
        mtl_map->insert(material["name"].toString(), result);
    }
    else if(QString::compare(type, QString("TransmissiveMaterial")) == 0)
    {
        std::shared_ptr<QImage> textureMap;
        std::shared_ptr<QImage> normalMap;
        Color3f Kt = ToVec3(material["Kt"].toArray());
        float eta = material["eta"].toDouble();
        if(material.contains(QString("textureMap"))) {
            QString img_filepath = local_path.toString().append(material["textureMap"].toString());
            textureMap = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("normalMap"))) {
            QString img_filepath = local_path.toString().append(material["normalMap"].toString());
            normalMap = std::make_shared<QImage>(img_filepath);
        }
        auto result = std::make_shared<TransmissiveMaterial>(Kt, eta, textureMap, normalMap);
        mtl_map->insert(material["name"].toString(), result);
    }
    else if(QString::compare(type, QString("GlassMaterial")) == 0)
    {
        std::shared_ptr<QImage> textureMapRefl;
        std::shared_ptr<QImage> textureMapTransmit;
        std::shared_ptr<QImage> normalMap;
        Color3f Kr = ToVec3(material["Kr"].toArray());
        Color3f Kt = ToVec3(material["Kt"].toArray());
        float eta = material["eta"].toDouble();
        if(material.contains(QString("textureMapRefl"))) {
            QString img_filepath = local_path.toString().append(material["textureMapRefl"].toString());
            textureMapRefl = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("textureMapTransmit"))) {
            QString img_filepath = local_path.toString().append(material["textureMapTransmit"].toString());
            textureMapTransmit = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("normalMap"))) {
            QString img_filepath = local_path.toString().append(material["normalMap"].toString());
            normalMap = std::make_shared<QImage>(img_filepath);
        }
        auto result = std::make_shared<GlassMaterial>(Kr, Kt, eta, textureMapRefl, textureMapTransmit, normalMap);
        mtl_map->insert(material["name"].toString(), result);
    }
    else if(QString::compare(type, QString("PlasticMaterial")) == 0)
    {
        std::shared_ptr<QImage> roughnessMap;
        std::shared_ptr<QImage> textureMapDiffuse;
        std::shared_ptr<QImage> textureMapSpecular;
        std::shared_ptr<QImage> normalMap;
        Color3f Kd = ToVec3(material["Kd"].toArray());
        Color3f Ks = ToVec3(material["Ks"].toArray());
        float roughness = material["roughness"].toDouble();
        if(material.contains(QString("roughnessMap"))) {
            QString img_filepath = local_path.toString().append(material["roughnessMap"].toString());
            roughnessMap = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("textureMapDiffuse"))) {
            QString img_filepath = local_path.toString().append(material["textureMapDiffuse"].toString());
            textureMapDiffuse = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("textureMapSpecular"))) {
            QString img_filepath = local_path.toString().append(material["textureMapSpecular"].toString());
            textureMapSpecular = std::make_shared<QImage>(img_filepath);
        }
        if(material.contains(QString("normalMap"))) {
            QString img_filepath = local_path.toString().append(material["normalMap"].toString());
            normalMap = std::make_shared<QImage>(img_filepath);
        }
        auto result = std::make_shared<PlasticMaterial>(Kd, Ks, roughness, roughnessMap, textureMapDiffuse, textureMapSpecular, normalMap);
        mtl_map->insert(material["name"].toString(), result);
    }
    else
    {
        std::cout << "Could not parse the material!" << std::endl;
        return false;
    }

    return true;
}

Camera JSONReader::LoadCamera(QJsonObject& camera)
{
    Camera result;
    if(camera.contains(QString("target"))) result.ref = ToVec3(camera["target"].toArray());
    if(camera.contains(QString("eye"))) result.eye = ToVec3(camera["eye"].toArray());
    if(camera.contains(QString("worldUp"))) result.world_up = ToVec3(camera["worldUp"].toArray());
    if(camera.contains(QString("width"))) result.width = camera["width"].toDouble();
    if(camera.contains(QString("height"))) result.height = camera["height"].toDouble();
    if(camera.contains(QString("fov"))) result.fovy = camera["fov"].toDouble();
    if(camera.contains(QString("nearClip"))) result.near_clip = camera["nearClip"].toDouble();
    if(camera.contains(QString("farClip"))) result.far_clip = camera["farClip"].toDouble();
    if(camera.contains(QString("lensRadius"))) result.lensRadius = camera["lensRadius"].toDouble();
    if(camera.contains(QString("focalDistance"))) result.focalDistance = camera["focalDistance"].toDouble();

    result.RecomputeAttributes();
    return result;
}

Transform JSONReader::LoadTransform(QJsonObject &transform)
{
    Vector3f t, r, s;
    s = Vector3f(1,1,1);
    if(transform.contains(QString("translate"))) t = ToVec3(transform["translate"].toArray());
    if(transform.contains(QString("rotate"))) r = ToVec3(transform["rotate"].toArray());
    if(transform.contains(QString("scale"))) s = ToVec3(transform["scale"].toArray());
    return Transform(t, r, s);
}

std::shared_ptr<Medium> JSONReader::LoadMedium( QJsonObject &medium )
{
    QString type;
    std::shared_ptr<Medium> m;

    //First check what type of medium we're supposed to load
    if(medium.contains(QString("type"))) type = medium["type"].toString();

    Float sigmaA = static_cast< float >(medium["sigmaA"].toDouble());
    Float sigmaS = static_cast< float >(medium["sigmaS"].toDouble());
    Float g = static_cast< float >(medium["g"].toDouble());

    if(QString::compare(type, QString("HomogeneousMedium")) == 0)
    {
        m = std::make_shared<HomogeneousMedia> (sigmaA, sigmaS, g);
    }
    else if(QString::compare(type, QString("HeterogeneousMedium")) == 0)
    {
        m = std::make_shared<HeterogeneousMedia> (sigmaA, sigmaS, g);
    }
    else
    {
        std::cout << "Could not parse the medium!" << std::endl;
    }

    return m;
}

glm::vec3 JSONReader::ToVec3(const QJsonArray &s)
{
    glm::vec3 result(s.at(0).toDouble(), s.at(1).toDouble(), s.at(2).toDouble());
    return result;
}

glm::vec3 JSONReader::ToVec3(const QStringRef &s)
{
    glm::vec3 result;
    int start_idx;
    int end_idx = -1;
    for(int i = 0; i < 3; i++){
        start_idx = ++end_idx;
        while(end_idx < s.length() && s.at(end_idx) != QChar(' '))
        {
            end_idx++;
        }
        result[i] = s.mid(start_idx, end_idx - start_idx).toFloat();
    }
    return result;
}

