#include "photons.h"

void Photons::run()
{
    shoot(); //using a proxy function to be able to easily debug by simply calling shoot instead of run
}

KdTree* Photons::createTree(KdTree* root)
{
    shootPhotons();
    constructTree(root);
}

void Photons::shootPhotons()
{


    for(int j=0; j<100; j++) //done to make it easy to convert to a parallel structure
    {
        //for every thread that is created, store the photons it is processing in a
        //separate std::vector and collate them all into one big list later
        run();
    }
}

void Photons::shoot()
{
    std::vector<Photon> threadPhotonListDirect;
    std::vector<Photon> threadPhotonListIndirect;
    std::vector<Photon> threadPhotonListCaustic;

    int numLights = scene->lights.size();
    int n1 = numPhotons/numLights;
    if(n1<1000)
    {
        n1 = 1000;
    }
    int n2 = (int)std::ceil(n1/100);

    for(int i=0; i<numLights; i++)
    {
        for(int k=0; i<n2; i++)
        {
            //use warp functions to generate a ray from the light into the scene
            WarpFunctions::squareToHemisphereCosine(sampler->Get2D());
            Vector3f origin = Vector3f(0.0f);
            Vector3f dir = Vector3f(0.0f);
            Ray ray = Ray(origin, dir);
            shootPhotonsHelper(*scene, sampler, recursionLimit, ray, i,
                               threadPhotonListDirect,
                               threadPhotonListIndirect,
                               threadPhotonListCaustic);
        }
    }
}

void Photons::shootPhotonsHelper(const Scene& scene, std::shared_ptr<Sampler> sampler,
                                 int depth, Ray& ray, int& lightIndex,
                                 std::vector<Photon>threadPhotonsDirect,
                                 std::vector<Photon>threadPhotonsIndirect,
                                 std::vector<Photon>threadPhotonsCaustic )
{
    Color3f accumulatedRayColor = Color3f(0.0f); //accumulated ray color
    Color3f accumulatedThroughputColor = Color3f(1.0f); //accumulated throughput color

    //keep track of energy and ray globally
    Vector3f woW = -ray.direction;
    bool flag_Terminate = false;
    bool flag_CameFromSpecular = false;
    bool flag_Hit_Specular = false;
    Ray r = ray;

    while( depth > 0 && !flag_Terminate)
    {
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

        if( depth == recursionLimit  || flag_CameFromSpecular )
        {
//            directLightingColor += intersection.Le(woW);
            //use light index to get Le
            //create a photon and add it to the Direct Light Map
        }

        intersection.ProduceBSDF();
        int randomLightIndex = -1;

        //Add photon creation bits using existing information

        //Direct Lighting

//        Vector3f wiW_Direct_Light;
//        float pdf_Direct_Light;
//        Point2f xi_Direct_Light = sampler->Get2D();
//        Color3f LTE_Direct_Light = Color3f(0.0f);

        //BSDF based Direct Lighting
//        Vector3f wiW_BSDF_Light;
//        float pdf_BSDF_Light;
//        Point2f xi_BSDF_Light = sampler->Get2D();
//        Color3f LTE_BSDF_Light = Color3f(0.0f);
        BxDFType sampledType;
//        int randomLightIndex_bsdf = -1;

        //MIS

        //Indirect Lighting
        Vector3f wiW_BSDF_Indirect;
        float pdf_BSDF_Indirect;
        Point2f xi_BSDF_Indirect = sampler->Get2D();

        flag_CameFromSpecular = false;
        if(flag_Hit_Specular) //if( (sampledType & BSDF_SPECULAR) == BSDF_SPECULAR )
        {
            flag_CameFromSpecular = true;
        }

        directLightingColor *= accumulatedThroughputColor;

        //f term
        Color3f f_BSDF_Indirect = intersection.bsdf->Sample_f(woW, &wiW_BSDF_Indirect, xi_BSDF_Indirect,
                                                              &pdf_BSDF_Indirect, BSDF_ALL, &sampledType);

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

KdTree* Photons::constructTree(KdTree* root)
{

}
