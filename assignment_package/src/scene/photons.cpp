#include "photons.h"

void Photons::run()
{
    shoot(); //using a proxy function to be able to easily debug by simply calling shoot instead of run
}

KdNode* Photons::createTree(KdTree*& kdTree)
{
    shootPhotons();
    kdTree = new KdTree(*scene, photonListIndirect, photonListCaustic);
    KdNode* root = kdTree->root;
    return root;
}

void Photons::shootPhotons()
{
    numPhotons = 1000;
    for(int j=0; j<100; j++) //done to make it easy to convert to a parallel structure
    {
        //for every thread that is created, store the photons it is processing in a
        //separate std::vector and collate them all into one big list later

        run();
//        shoot();
    }
}

void Photons::shoot()
{
    std::vector<Photon*>* threadPhotonListDirect = new std::vector<Photon*>;
    std::vector<Photon*>* threadPhotonListIndirect = new std::vector<Photon*>;
    std::vector<Photon*>* threadPhotonListCaustic = new std::vector<Photon*>;

    int numLights = scene->lights.size();
    int n1 = numPhotons/numLights;
    if(n1<1000)
    {
        n1 = 1000;
    }
    int n2 = (int)std::ceil(n1/100);

    for(int i=0; i<numLights; i++)
    {
        float alpha = scene->lights.at(i)->LightEmitted().length();//confirm with adam
        for(int k=0; k<n2; k++)
        {
            //use warp functions to generate a ray from the light into the scene

            Ray ray = scene->lights.at(i)->createPhotonRay(sampler);
            shootPhotonsHelper(*scene, sampler, recursionLimit, ray, i, alpha,
                               threadPhotonListDirect,
                               threadPhotonListIndirect,
                               threadPhotonListCaustic);
        }
    }

    //May want to look at thread safe functions --> they can prevent 2 threads
    //from writing to the same photonList
    photonListIndirect.push_back(threadPhotonListIndirect);
    photonListCaustic.push_back(threadPhotonListCaustic);
}

void Photons::shootPhotonsHelper(const Scene& scene, std::shared_ptr<Sampler> sampler,
                                 int depth, Ray& ray, int& lightIndex, float alpha,
                                 std::vector<Photon *> *threadPhotonsDirect,
                                 std::vector<Photon *> *threadPhotonsIndirect,
                                 std::vector<Photon *> *threadPhotonsCaustic )
{
    Color3f accumulatedRayColor = Color3f(0.0f); //accumulated ray color
    Color3f accumulatedThroughputColor = Color3f(1.0f); //accumulated throughput color

    //keep track of energy and ray globally
    Vector3f woW = -ray.direction;
    bool flag_Terminate = false;
    bool flag_CameFromCausticProducer = false;
    bool flag_Hit_CausticProducer = false;
    Ray r = ray;

    while( depth > 0 && !flag_Terminate)
    {
        //Photons are added to their respective maps as they hit surfaces, disregarding the first intersection with the scene
        //Direct Lighting is handled in the actual integrator as you will get better results (its easier and less biased to do BSDF based
        //direct lighting and then MIS)
        float probability = sampler->Get1D(); //russian roulette random number
        Color3f directLightingColor = Color3f(0.0);

        Intersection intersection = Intersection();
        bool objhit = scene.Intersect(r, &intersection);

        if(!objhit)
        {
            return; // this photon has left the scene and thus wont be added to any map,
                    //nor can it produce photons that can be added to maps
        }

        if(objhit && intersection.objectHit->material == nullptr)
        {
            //it hit another light
            // since we can't produce a bsdf we dont know where to go from here
            // so exit function
            return;
        }

        intersection.ProduceBSDF();
        int randomLightIndex = -1;

        //Indirect Lighting
        Vector3f wiW_BSDF_Indirect;
        float pdf_BSDF_Indirect;
        Point2f xi_BSDF_Indirect = sampler->Get2D();
        BxDFType sampledType;

        if( depth == recursionLimit )
        {
            directLightingColor += scene.lights.at(lightIndex)->LightEmitted();

            //Since this is the first intersection with the scene of a photon shot froma light source. It is the
            //equivalent of direct lighting, however we are handling direct lighting in the integrator. So we
            //skip the first bounce.

            intersection.bsdf->Sample_f(woW, &wiW_BSDF_Indirect, xi_BSDF_Indirect,
                                        &pdf_BSDF_Indirect, BSDF_ALL, &sampledType);

            Ray ray_skipfirstbounce = intersection.SpawnRay(wiW_BSDF_Indirect);
            woW = -ray_skipfirstbounce.direction;
            r = ray_skipfirstbounce;
            depth--;
            continue;
        }

        flag_CameFromCausticProducer = false;
        if(flag_Hit_CausticProducer) //if( (sampledType & BSDF_SPECULAR) == BSDF_SPECULAR )
        {
            flag_CameFromCausticProducer = true;
        }
        flag_Hit_CausticProducer = false; // comment out to strengthen the effect of caustics

        directLightingColor *= accumulatedThroughputColor;

        //f term
        Color3f f_BSDF_Indirect = intersection.bsdf->Sample_f(woW, &wiW_BSDF_Indirect, xi_BSDF_Indirect,
                                                              &pdf_BSDF_Indirect, BSDF_ALL, &sampledType);

        if( sampledType & BSDF_SPECULAR )
        {
            flag_Hit_CausticProducer = true;
        }

        if(pdf_BSDF_Indirect != 0.0f)
        {
            //No Li term per se, this is accounted for via accumulatedThroughputColor

            //absDot Term
            float absDot_BSDF_Indirect = AbsDot(wiW_BSDF_Indirect, intersection.normalGeometric);

            //LTE term
            accumulatedThroughputColor *= f_BSDF_Indirect * absDot_BSDF_Indirect / pdf_BSDF_Indirect;
        }

        flag_Terminate = RussianRoulette(accumulatedThroughputColor, probability, depth); //can change accumulatedThroughput
        accumulatedRayColor += directLightingColor;

        if(!flag_Hit_CausticProducer && flag_CameFromCausticProducer)
        {
            //store photon in caustic map
            Photon* p = new Photon(intersection.point, woW, alpha, accumulatedRayColor);
            threadPhotonsCaustic->push_back(p);
        }
        else
        {
            //store photon in diffuse map
            Photon* p = new Photon(intersection.point, woW, alpha, accumulatedRayColor);
            threadPhotonsIndirect->push_back(p);
        }

        Ray n_ray_BSDF_Indirect = intersection.SpawnRay(wiW_BSDF_Indirect);
        woW = -n_ray_BSDF_Indirect.direction;
        r = n_ray_BSDF_Indirect;
        depth--;
    }
}

bool Photons::RussianRoulette(Color3f& energy, float probability, int depth) const
{
    //return true to end recursive call to Li
    if(depth <= (recursionLimit - 3))
    {
        float highest_energy_component = std::max( std::max(energy.x, energy.y), energy.z );
        energy = energy/highest_energy_component;
        if(highest_energy_component < probability)
        {
            return true;
        }
    }

    return false;
}
