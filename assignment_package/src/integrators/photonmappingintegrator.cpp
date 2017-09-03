#include "photonmappingintegrator.h"

Color3f PhotonMappingIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    //shoot ray into scene, it intersects something in the scene; you shoot a direct ray towards a light source
    //and get a color value, you get the global illumination color value by search for photons in the viscinity of
    //the intersection in some search radius, blur the resulting photon colors; carry out MIS on the resulting 2 colors
    //and repeat per pixel


    int numLights = scene.lights.size();
    if(numLights == 0)
    {
        return Color3f(0.0f);
    }

    Color3f accumulatedRayColor = Color3f(0.0f); //accumulated ray color
    Color3f accumulatedThroughputColor = Color3f(1.0f); //accumulated throughput color

    //keep track of energy and ray globally
    Vector3f woW = -ray.direction;
    bool flag_BSDF = true;
    bool flag_CameFromSpecular = false;
    bool flag_Hit_Specular = false;
    Ray r = ray;

    float probability = sampler->Get1D(); //russian roulette random number
    Color3f directLightingColor = Color3f(0.0);

    Intersection intersection = Intersection();
    bool objhit = scene.Intersect(r, &intersection);

    Vector3f wiW_Direct_Light;
    float pdf_Direct_Light;
    Point2f xi_Direct_Light = sampler->Get2D();
    Color3f LTE_Direct_Light = Color3f(0.0f);

    Vector3f wiW_BSDF_Light;
    float pdf_BSDF_Light;
    Point2f xi_BSDF_Light = sampler->Get2D();
    Color3f LTE_BSDF_Light = Color3f(0.0f);
    BxDFType sampledType;

    if(!objhit)
    {
        flag_BSDF = false; // there will be no BSDF
        bool flag_enteredIf = false;
        for(int i=0; i<scene.lights.size(); i++)
        {
            if(scene.lights.at(i)->flag_infiniteLight)
            {
                flag_enteredIf = true;
                accumulatedRayColor += scene.lights.at(i)->Sample_Li(intersection, xi_Direct_Light,
                                                                    &woW, &pdf_Direct_Light);
            }
        }

        return accumulatedRayColor;
    }

    if(objhit && intersection.objectHit->material == nullptr)
    {
        flag_BSDF = false; // there will be no BSDF
    }

    intersection.ProduceBSDF();
    int randomLightIndex = -1;

    //-------------------------- Specular Surface -------------------------

    while( (intersection.bsdf->BxDFsMatchingFlags(BSDF_SPECULAR) == 1) &&
           (intersection.bsdf->BxDFsMatchingFlags(BSDF_ALL) == 1) )
    {
        //reflect ray off of specular surface until it hits a non-specular surface
        Vector3f wiW_indirect;
        float pdf_BSDF_Indirect;
        Point2f xi_BSDF_Indirect = sampler->Get2D();

        intersection.bsdf->Sample_f(woW, &wiW_indirect, xi_BSDF_Indirect,
                                    &pdf_BSDF_Indirect, BSDF_ALL, &sampledType);

        Ray n_ray_BSDF_Indirect = intersection.SpawnRay(wiW_indirect);
        woW = -n_ray_BSDF_Indirect.direction;
        r = n_ray_BSDF_Indirect;

        intersection = Intersection();
        objhit = scene.Intersect(r, &intersection);

        if(!objhit)
        {
            flag_BSDF = false; // there will be no BSDF
            bool flag_enteredIf = false;
            for(int i=0; i<scene.lights.size(); i++)
            {
                if(scene.lights.at(i)->flag_infiniteLight)
                {
                    flag_enteredIf = true;
                    accumulatedRayColor += scene.lights.at(i)->Sample_Li(intersection, xi_Direct_Light,
                                                                        &woW, &pdf_Direct_Light);
                }
            }

            return accumulatedRayColor;
        }

        if(objhit && intersection.objectHit->material == nullptr)
        {
            flag_BSDF = false; // there will be no BSDF
            break;
        }
        else
        {
            intersection.ProduceBSDF();
        }
    }

    //----------------------- Actual Direct Lighting ----------------------

    if(flag_BSDF)
    {
        //Li term - not recursive, cause direct lighting
        randomLightIndex = std::min((int)std::floor(sampler->Get1D()*numLights), numLights - 1);
        Color3f li_Direct_Light = scene.lights.at(randomLightIndex)->Sample_Li(intersection, xi_Direct_Light,
                                                                            &wiW_Direct_Light, &pdf_Direct_Light);

        //f term term
        Color3f f_Direct_Light = intersection.bsdf->f(woW, wiW_Direct_Light, BSDF_ALL);

        if( pdf_Direct_Light != 0.0f )
        {
            //absDot term
            Ray n_ray_Direct_Light = intersection.SpawnRay( wiW_Direct_Light );
            float absDot_Direct_Light = AbsDot(wiW_Direct_Light, intersection.normalGeometric);

            //visibility test for direct lighting
            Intersection isx;
            objhit = scene.Intersect(n_ray_Direct_Light, &isx);

            //LTE calculation for Direct Lighting
            if(objhit && isx.objectHit->areaLight == scene.lights.at(randomLightIndex))
            {
                LTE_Direct_Light = f_Direct_Light * li_Direct_Light * absDot_Direct_Light / pdf_Direct_Light;
            }
        }
    }

    //----------------------- BSDF based Direct Lighting ------------------

    if(flag_BSDF)
    {
        flag_Hit_Specular = true;

        //if the object is pure specular
        if( !((intersection.bsdf->BxDFsMatchingFlags(BSDF_SPECULAR) == 1) &&
              (intersection.bsdf->BxDFsMatchingFlags(BSDF_ALL) == 1)) )
        {
            flag_Hit_Specular = false;
            //f term
            Color3f f_BSDF_Light = intersection.bsdf->Sample_f(woW, &wiW_BSDF_Light, xi_BSDF_Light,
                                                               &pdf_BSDF_Light, BSDF_ALL, &sampledType);

            if(pdf_BSDF_Light != 0.0f)
            {
                //absDot Term
                float absDot_BSDF_Light = AbsDot(wiW_BSDF_Light, intersection.normalGeometric);

                //visibility test for bsdf based direct lighting; this also tests if the bsdf
                //direct lighting ray actually hit the selected light used for actual direct
                //lighting (Light important sampling based direct lighting)
                Ray n_ray_BSDF_Light = intersection.SpawnRay(wiW_BSDF_Light);
                Intersection isx_bsdf;
                objhit = scene.Intersect(n_ray_BSDF_Light, &isx_bsdf);

                //LTE calculation for BSDF based Direct Lighting
                if(objhit)
                {
                    if(isx_bsdf.objectHit->areaLight != scene.lights.at(randomLightIndex))
                    {
                        LTE_BSDF_Light = Color3f(0.0f);
                    }
                    else
                    {
                        //Li term - not recursive, cause direct lighting; emitted Light
                        //Color3f li_BSDF_Light = scene.lights.at(randomLightIndex_bsdf)->L( isx_bsdf, -wiW_BSDF_Light );
                        Color3f li_BSDF_Light = scene.lights.at(randomLightIndex)->L( isx_bsdf, -wiW_BSDF_Light );

                        //LTE term
                        LTE_BSDF_Light = f_BSDF_Light * li_BSDF_Light * absDot_BSDF_Light / pdf_BSDF_Light;
                    }
                }
                else
                {
                    for(int i=0; i<scene.lights.size(); i++)
                    {
                        if(scene.lights.at(i)->flag_infiniteLight)
                        {
                            Color3f li_BSDF_Light = scene.lights.at(i)->L( isx_bsdf, -wiW_BSDF_Light );
                            LTE_BSDF_Light = li_BSDF_Light;
                        }
                    }
                }
            }
        }
    }

    //----------------------- MIS -----------------------
    //MIS can only work on one type of light  energy, and so we use MIS for direct lighting only
    float weight_BSDF_Light = 0.0;
    float weight_Direct_Light = 0.0;

    if(flag_BSDF)
    {
        if(randomLightIndex != -1)
        {
            weight_BSDF_Light = PowerHeuristic( 1, pdf_BSDF_Light, 1, (scene.lights.at(randomLightIndex)->Pdf_Li(intersection, wiW_BSDF_Light)) );
            weight_Direct_Light = PowerHeuristic( 1, pdf_Direct_Light, 1, intersection.bsdf->Pdf(woW, wiW_Direct_Light, BSDF_ALL) );
        }

        Color3f weighted_BSDF_Light_color = LTE_BSDF_Light * weight_BSDF_Light;
        Color3f weighted_Direct_Light_color = LTE_Direct_Light * weight_Direct_Light;

        directLightingColor = (weighted_BSDF_Light_color + weighted_Direct_Light_color) * (float)numLights;
    }

    //----------------------- Indirect Lighting (Global Illumination) -----
    Vector3f wiW_BSDF_Indirect;
    float pdf_BSDF_Indirect;
    Point2f xi_BSDF_Indirect = sampler->Get2D();

    directLightingColor += intersection.Le(woW);

    KdNode* node = scene.kdTree->FindNode(intersection.point);
    std::vector<KdNode*> nodeList;
    scene.kdTree->FindAllNeighborNodes(node, nodeList, intersection.point, scene.kdTree->searchRadius);
    accumulatedThroughputColor = scene.kdTree->BlendPhotons(nodeList, intersection.point); //averaged photon color

    directLightingColor *= accumulatedThroughputColor;
    accumulatedRayColor += directLightingColor;

    return accumulatedRayColor;
}

void PhotonMappingIntegrator::GetIndirectRayDirectionfromBSDF(Vector3f& woW, Vector3f& wiW) const
{

//    BxDFType sampledType;
//    //-------------------------------------------------------------------------
//    //choose which BxDF to sample
//    int matchingComps = BSDF::BxDFsMatchingFlags(BSDF_ALL);
//    if(matchingComps == 0)
//    {
//        *pdf = 0; //because we don't want to sample it???
//        return Color3f(0.0f);
//    }
//    int comp = std::min((int)std::floor(xi[0]*matchingComps), matchingComps - 1);
//    // random bxdf choosing

//    BxDF *bxdf = nullptr;
//    int count = comp;

//    for(int i=0; i<numBxDFs ;++i)
//    {
//        if(bxdfs[i]->MatchesFlags(type) && count-- == 0)
//            // count is only decremented when a bxdfs[i] == mathcing flag
//        {
//            bxdf = bxdfs[i];
//            break;
//        }
//    }

//    //-------------------------------------------------------------------------
//    //Remap BxDF sample u to [0,1)
//    Point2f uRemapped = Point2f(std::min(xi[0] * matchingComps - comp, OneMinusEpsilon), xi[1]);
//    //so that we can sample the entire hemisphere and not just a subsection of it;
//    //xi[1] is independent of the bxdf, and so the bxdf cant bias it

//    //-------------------------------------------------------------------------
//    //Sample chosen BxDF
//    Vector3f wo = worldToTangent * woW;
//    Vector3f wi = Vector3f (0.0f);
//    *pdf = 0.0f;
//    if(sampledType)
//    {
//        //this is because its a recursive call; we are shooting a ray that bounces off
//        //many materials on its, journey, so if it was sampled before, then sampledType
//        //will exist and we have to reequate it to the type of the material hit
//        *sampledType = type;
//    }

//    bxdf->Sample_f(wo, &wi, uRemapped, pdf, sampledType);
//    wiW = tangentToWorld * wi;
}

float PhotonMappingIntegrator::BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf) const
{
    float denominator = (fPdf*nf + gPdf*ng);
    if( fequal( denominator, 0.0f) )
    {
        return 0.0f;
    }
    return (nf*fPdf)/denominator;
}

float PhotonMappingIntegrator::PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf) const
{
    float denominator = (fPdf*nf*fPdf*nf + gPdf*ng*gPdf*ng);
    if( fequal( denominator, 0.0f) )
    {
        return 0.0f;
    }
    return std::pow((nf*fPdf), 2)/denominator;
}

bool PhotonMappingIntegrator::RussianRoulette(Color3f& energy, float probability, int depth) const
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
