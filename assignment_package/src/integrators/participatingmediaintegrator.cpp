#include "participatingmediaintegrator.h"
#include <scene/bvh.h>

Color3f ParticipatingMediaIntegrator::Li(const Ray &ray, const Scene &scene, std::shared_ptr<Sampler> sampler, int depth) const
{
    Color3f accumulatedRayColor = Color3f(0.0f); //accumulated ray color
    Color3f accumulatedThroughputColor = Color3f(1.0f); //accumulated throughput color

    //keep track of energy and ray globally
    Vector3f woW = -ray.direction;
    bool flag_NoBSDF = false;
    bool flag_terminate = false;
    bool flag_CameFromSpecular = false;

    //RAY MARCHING Variables
    //camera position for initial ray marching
    Vector3f rayMarchOrigin = scene.camera.eye;
    Vector3f rayMarchEnd;
    Bounds3f scene_bounds = Bounds3f(Point3f(-50.0f,50.0f,-50.0f), Point3f(50.0f,50.0f,50.0f));

//    float dist = glm::distance(rayMarchOrigin, rayMarchEnd);
    float stepsize = 1.0f;
    float offset = 0.01f;
    float t = 0.0f;

    Ray r = ray;
    int numLights = scene.lights.size();
    if(numLights == 0)
    {
        return Color3f(0.0f);
    }

    while( depth > 0 && !flag_terminate)
    {
        float probability = sampler->Get1D(); //russian roulette random number
        Color3f directLightingColor = Color3f(0.0);

        Intersection intersection = Intersection();
        bool objhit = scene.Intersect(r, &intersection);        

        //----------------------- Volumetric Ray Marching ----------------------
        //use as a scaling factor and to change the color of accumulatedthroughput
        Color3f volumeColor = Color3f(0.0f); //only change it to 0.0f if medium exists, otherwise you are
                        //going to multiply accumulatedthroughput by volumeColor, giving you a black scene
        float accumulatedTransmittance = 1.0f;

        if(scene.medium != nullptr)
        {
            //ray march, move in by one step
            if(!objhit)
            {
                Vector3f dir = ray.direction;
                rayMarchEnd = Vector3f(dir.x*50.0f, dir.y*50.0f, dir.z*50.0f);
            }
            else
            {
                rayMarchEnd = intersection.point + glm::normalize(intersection.normalGeometric) * offset;
            }

            float dist = glm::distance(rayMarchOrigin, rayMarchEnd);

            while(accumulatedTransmittance>0.0f && t<(dist))
            {
                Vector3f rayMarchedPoint = rayMarchOrigin + t*glm::normalize(ray.direction);
                accumulatedTransmittance = scene.medium->Transmittance(rayMarchOrigin, rayMarchedPoint);
                Color3f pointColor = getLightingBasedColor(scene, sampler, numLights,
                                                    rayMarchedPoint, offset, stepsize, ray);

                volumeColor += pointColor*accumulatedTransmittance;
                t += stepsize;
            }

            t = 0.0f;
            rayMarchOrigin = rayMarchEnd;
        }

        Vector3f wiW_Direct_Light;
        float pdf_Direct_Light;
        Point2f xi_Direct_Light = sampler->Get2D();
        Color3f LTE_Direct_Light = Color3f(0.0f);

        bool flag_enteredIf = false;
        if(!objhit)
        {

            for(int i=0; i<scene.lights.size(); i++)
            {
                if(scene.lights.at(i)->flag_infiniteLight)
                {
                    flag_enteredIf = true;
                    Color3f li_Direct_Light = scene.lights.at(i)->Sample_Li(intersection,
                                                                            xi_Direct_Light,
                                                                            &wiW_Direct_Light,
                                                                            &pdf_Direct_Light);
                    LTE_Direct_Light = li_Direct_Light;
                }
            }
            if(!flag_enteredIf)
            {
                return volumeColor;
            }
        }

        if(intersection.objectHit->material == nullptr)
        {
            directLightingColor = Color3f(0.0f);
            flag_NoBSDF = true; // there will be no BSDF
        }

        intersection.ProduceBSDF();
        int randomLightIndex = -1;

        //----------------------- Actual Direct Lighting ----------------------
        if(flag_NoBSDF == true)
        {
            LTE_Direct_Light = Color3f(0.0f);
        }
        else
        {
            //Li term - not recursive, cause direct lighting
            randomLightIndex = std::min((int)std::floor(sampler->Get1D()*numLights), numLights - 1);
            Color3f li_Direct_Light = scene.lights.at(randomLightIndex)->Sample_Li(intersection,
                                                                                xi_Direct_Light,
                                                                                &wiW_Direct_Light,
                                                                                &pdf_Direct_Light);

            //f term term
            Color3f f_Direct_Light = intersection.bsdf->f(woW, wiW_Direct_Light, BSDF_ALL);

            if( pdf_Direct_Light == 0.0f )
            {
                LTE_Direct_Light = Color3f(0.0f);
            }
            else
            {
                //absDot term
                Ray n_ray_Direct_Light = intersection.SpawnRay( wiW_Direct_Light );
                float absDot_Direct_Light = AbsDot(wiW_Direct_Light, intersection.normalGeometric);

                //visibility test for direct lighting
                Intersection isx;
                objhit = scene.Intersect(n_ray_Direct_Light, &isx);

                //LTE calculation for Direct Lighting
                if(objhit && isx.objectHit->areaLight != scene.lights.at(randomLightIndex))
                {
                    LTE_Direct_Light = Color3f(0.0f);
                }
                else
                {
                    LTE_Direct_Light = f_Direct_Light * li_Direct_Light * absDot_Direct_Light / pdf_Direct_Light;
                }
            }
        }

        //----------------------- BSDF based Direct Lighting ------------------
        Vector3f wiW_BSDF_Light;
        float pdf_BSDF_Light;
        Point2f xi_BSDF_Light = sampler->Get2D();
        Color3f LTE_BSDF_Light = Color3f(0.0f);
        BxDFType sampledType;
        int randomLightIndex_bsdf = -1;

        if(flag_NoBSDF)
        {
            LTE_BSDF_Light = Color3f(0.0f);
        }
        else
        {
            //if the object is pure specular
            if( (intersection.bsdf->BxDFsMatchingFlags(BSDF_SPECULAR) == 1) &&
                (intersection.bsdf->BxDFsMatchingFlags(BSDF_ALL) == 1) )
            {
                LTE_BSDF_Light = Color3f(0.0f);
            }
            else
            {
                //f term
                Color3f f_BSDF_Light = intersection.bsdf->Sample_f(woW, &wiW_BSDF_Light, xi_BSDF_Light,
                                                                   &pdf_BSDF_Light, BSDF_ALL, &sampledType);

                if(pdf_BSDF_Light == 0.0f)
                {
                    LTE_BSDF_Light = Color3f(0.0f);
                }
                else
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

        if(flag_NoBSDF == false)
        {
            if(randomLightIndex != -1)
            {
                weight_BSDF_Light = PowerHeuristic( 1, pdf_BSDF_Light, 1, (scene.lights.at(randomLightIndex)->Pdf_Li(intersection, wiW_BSDF_Light) ));
                weight_Direct_Light = PowerHeuristic( 1, pdf_Direct_Light, 1, intersection.bsdf->Pdf( woW, wiW_Direct_Light, BSDF_ALL));
            }

            Color3f weighted_BSDF_Light_color = LTE_BSDF_Light * weight_BSDF_Light;
            Color3f weighted_Direct_Light_color = LTE_Direct_Light * weight_Direct_Light;

            directLightingColor = (weighted_BSDF_Light_color + weighted_Direct_Light_color) * (float)numLights;
        }

        //----------------------- Indirect Lighting (Global Illumination) -----
        Vector3f wiW_BSDF_Indirect;
        float pdf_BSDF_Indirect;
        Point2f xi_BSDF_Indirect = sampler->Get2D();

        if( depth == recursionLimit  || flag_CameFromSpecular )
        {
            directLightingColor += intersection.Le(woW);
        }

        directLightingColor *= accumulatedThroughputColor;

        if( flag_NoBSDF == false)
        {
            //f term
            Color3f f_BSDF_Indirect = intersection.bsdf->Sample_f(woW, &wiW_BSDF_Indirect, xi_BSDF_Indirect,
                                                                  &pdf_BSDF_Indirect, BSDF_ALL, &sampledType);

            if(pdf_BSDF_Indirect == 0.0f)
            {
                accumulatedThroughputColor = Color3f(0.0f);
            }
            else
            {
                //No Li term per se, this is accounted for via accumulatedThroughputColor

                //absDot Term
                float absDot_BSDF_Indirect = AbsDot(wiW_BSDF_Indirect, intersection.normalGeometric);

                //LTE term
                accumulatedThroughputColor *= f_BSDF_Indirect * absDot_BSDF_Indirect / pdf_BSDF_Indirect;
            }
        }

        flag_terminate = RussianRoulette(accumulatedThroughputColor, probability, depth); //can change accumulatedThroughput
        accumulatedRayColor += directLightingColor;
        accumulatedRayColor *= accumulatedTransmittance;
        accumulatedRayColor += volumeColor*Color3f(0.1f);//To account for the participating media
        //Scaling volumeColor cause it was oversaturating the scene

        if(intersection.objectHit->material == nullptr)
        {
            return accumulatedRayColor;
        }

        Ray n_ray_BSDF_Indirect = intersection.SpawnRay(wiW_BSDF_Indirect);
        woW = -n_ray_BSDF_Indirect.direction;
        r = n_ray_BSDF_Indirect;
        depth--;
    }

    return accumulatedRayColor;
}

Color3f ParticipatingMediaIntegrator::getLightingBasedColor(const Scene &scene, std::shared_ptr<Sampler> sampler, int &numLights,
                              Point3f &point, float &offset, float &stepsize, const Ray &ray) const
{
    //Use the BSDF of the Mesh that contains the volume, So every point in the media has a BSDF
    Color3f indirectLightingOnMediaPoint = IndirectLightingRayMarched(scene, sampler, numLights, point, offset, stepsize, ray);

    return indirectLightingOnMediaPoint;
}

Color3f ParticipatingMediaIntegrator::DirectLightingLISRayMarched(const Scene &scene, std::shared_ptr<Sampler> sampler, int &numLights,
                                 Point3f &point, float &offset, float &stepsize) const
{
    int randLightIndex = std::min((int)std::floor(sampler->Get1D()*numLights), numLights - 1);
    Vector3f lightPos = glm::vec3( scene.lights.at(randLightIndex)->transform.T() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) );
    Vector3f dirToLight = lightPos - point;
    dirToLight = glm::normalize(dirToLight);
    float distToLight = glm::distance(lightPos, point);
    float t = 0.0f;
    Ray rayMediaToLight = Ray(point, dirToLight);
    Intersection intersectionMediaToLight = Intersection();
    bool objhitMediaToLight = scene.Intersect(rayMediaToLight, &intersectionMediaToLight);

    if(objhitMediaToLight)
    {
        if(intersectionMediaToLight.objectHit->areaLight != scene.lights.at(randLightIndex))
        {
            return Color3f(0.0f);
        }
        else
        {
            //ray march
            float accumulatedTransmittance = scene.medium->Transmittance(point, lightPos);

            return ( Color3f(accumulatedTransmittance) * scene.medium->RetrieveColor() *
                     scene.lights.at(randLightIndex)->L(intersectionMediaToLight, rayMediaToLight.direction) );
        }
    }
    else
    {
        return Color3f(0.0f, 0.0f, 0.0f);
    }
}

Color3f ParticipatingMediaIntegrator::IndirectLightingRayMarched(const Scene &scene, std::shared_ptr<Sampler> sampler, int &numLights,
                                                                 Point3f &point, float &offset, float &stepsize, const Ray &ray) const
{
    float m = 25; //number of samples for
    Color3f accumulatedScatteredColor = Color3f(0.0f,0.0f,0.0f);

    for(int i=0; i<m; i++)
    {
        //scattering changes the ray direction based on density
        Vector3f w_dash = glm::normalize( WarpFunctions::squareToSphereUniform(sampler->Get2D()) );
        //there is no need to transform w_dash by a transform matrix cause its just a direction vector
        //and the only thing thats happened is that we have moved to a point in space that isn't the origin

        Color3f scatteredColor = Color3f( scene.medium->PhaseFunction(-ray.direction, w_dash) );
        Intersection isectScatter = Intersection();
        Ray r = Ray(point, w_dash);
        bool objhit = scene.Intersect(r, &isectScatter);

        if(objhit && isectScatter.objectHit->material == nullptr)
        {
            float pdf_Scatter = Inv4Pi;
            Color3f intersectionColor = isectScatter.Le(-w_dash);

            float accumulatedTransmittanceScatter = 1.0f;
            float t = 0.0f;
            Vector3f rayMarchOrigin = point;
            Vector3f rayMarchEnd = isectScatter.point + glm::normalize(isectScatter.normalGeometric);
            float dist = glm::distance(rayMarchEnd, rayMarchOrigin);

            accumulatedTransmittanceScatter = scene.medium->Transmittance(rayMarchOrigin, rayMarchEnd);

            scatteredColor *= accumulatedTransmittanceScatter * intersectionColor;
            scatteredColor /= pdf_Scatter;

            accumulatedScatteredColor += scatteredColor;
        }
    }
    accumulatedScatteredColor /= m;
    accumulatedScatteredColor *= scene.medium->sigmaS;

    return accumulatedScatteredColor;
}

float ParticipatingMediaIntegrator::BalanceHeuristic(int nf, Float fPdf, int ng, Float gPdf) const
{
    float denominator = (fPdf*nf + gPdf*ng);
    if( fequal( denominator, 0.0f) )
    {
        return 0.0f;
    }
    return (nf*fPdf)/denominator;
}

float ParticipatingMediaIntegrator::PowerHeuristic(int nf, Float fPdf, int ng, Float gPdf) const
{
    float denominator = (fPdf*nf*fPdf*nf + gPdf*ng*gPdf*nf);
    if( fequal( denominator, 0.0f) )
    {
        return 0.0f;
    }
    return std::pow((nf*fPdf), 2)/denominator;
}

bool ParticipatingMediaIntegrator::RussianRoulette(Color3f& energy, float probability, int depth) const
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
