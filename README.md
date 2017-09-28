CPU Monte Carlo Path Tracer
===========================

* Name: Aman Sachan
* Built from scratch for CIS 561: Advanced Computer Graphics at the Universoty of Pennsylvania

------------
#README

[![](./readme_images/PathTracerVimeoLink.png)](https://vimeo.com/235422890)

## Introduction
  
  Monte Carlo Path tracing is a rendering technique that aims to represent global illumination as accurately as possible with respect to reality. The algorithm does this by approximating the integral over all the illuminance arriving at a point in the scene. A portion of this illuminance will go towards the viewpoint camera, and this portion of illuminance is determined by a surface reflectance function (BRDF). This integration procedure is repeated for every pixel in the output image. When combined with physically accurate models of surfaces, accurate models light sources, and optically-correct cameras, path tracing can produce still images that are indistinguishable from photographs.

  Path tracing naturally simulates many effects that have to be added as special cases for other methods (such as ray tracing or scanline rendering); these effects include soft shadows, depth of field, motion blur, caustics, ambient occlusion, and indirect lighting, etc.

  Due to its accuracy and unbiased nature, path tracing is used to generate reference images when testing the quality of other rendering algorithms. In order to get high quality images from path tracing, a large number of rays must be traced to avoid visible noisy artifacts.

## Features Implemented:

### Full Lighting Integrator
### Multiple Important Sampling
### Volumetric Integrator
### Bounding Volume Hierarchy
### Multi Threading

### Various BSDFs
#### Microfacet Model

### Various Lights

### Direct Lighting Integrator
### Naive Lighting Integrator

### Complex Implicit Surfaces
### Thin Lens Camera Model

### Power and Balance Heuristics

They are used to weight the energy contributions of rays which is handy during Multiple Important Sampling. Power and Balance Heuristics are just different rules of thumb for this weighting.

### Russian Roullete Termination

It is a technique employed to cut short the ray bounce depth once the energy being returned from the ray falls below a randomised threshold. The loss in energy is compensated by scaling up the existing energy value by an appropriate amount. When averaged over a bunch of samples, the loss in accuracy of the render is barely noticeable if at all.

### Sampling Techniques

A variety of sampling techniques were implemented for the various primitives and shapes that the path tracer deals with. All of these sampling techniques involved warping samples on a square plane to the shape in consideration. This way, I didn't have to implement a whole new sampling technique for every shape. This warping technique also allows for the introduction of biases such as cosine weighted sampling, in a controllable manner and from a common starting point.

The samples on the square plane are generated in a "stratified uniform grid", which basically means that you generate uniform points for each grid cell of the square plane and then randomly jitter the samples within the grid cell. This avoids the banding and distinct boundaries 
(think distinct bands in a shadow rather than a smooth gradient) that are a pitfall of uniform grid sampling. It also avoids the possible clumping of samples that can happen in purely randomized sampling, which can introduce unwanted bias in the image and sometimes still produce artifacts like the banding issue of uniform grid sampling.

### Intersection Testing with various primitives

The Path Tracer handles various primitives such as implicit surfaces, triangles for meshes, and the basic ones like cubes, spheres, discs, and squareplanes. Intersection testing for primitives happens in tangent space where we can greatly simplify the equations for determining an intersection, because we can guarantee things like the primitive being at the origin, facing the positive z direction, having a fixed scale 
(i.e. not being scale distorted), etc. For this reason the ray that will be used for intersection testing is transformed into tangent space for the intersection test and then converted back into world space right after.

## Reference: 

[Physically Based Rendering, Third Edition](http://www.pbrt.org/)