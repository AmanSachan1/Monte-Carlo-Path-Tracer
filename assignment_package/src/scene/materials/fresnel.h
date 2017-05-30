#pragma once
#include <globals.h>

// Pronounced fruh-nel, NOT fress-nel
// It's the last name of Augustin-Jean Fresnel, a French physicist who
// studied the theory of wave optics

// An abstract class for computing Fresnel reflection coefficients
// This simply means "how much should we weight the reflected color
// of this surface compared to its other BxDF colors?"
class Fresnel {
 public:
   // Fresnel Interface
   virtual ~Fresnel(){}
   virtual Color3f Evaluate(float cosI) const = 0;
};

// A Fresnel class that always returns 100% reflection regardless of ray direction
// While not completely physically accurate, it is useful for testing BRDF
// implementations
class FresnelNoOp : public Fresnel {
  public:
    Color3f Evaluate(float) const { return Color3f(1.0f); }
};

class FresnelNoReflect : public Fresnel {
public:
    Color3f Evaluate(float) const { return Color3f(0.0f); }
};

// A Fresnel class that computes the Fresnel reflection coefficient for
// dielectric surfaces such as glass, water, or air.
class FresnelDielectric : public Fresnel {
  public:
    FresnelDielectric(float etaI, float etaT) : etaI(etaI), etaT(etaT) {}
    Color3f Evaluate(float cosThetaI) const;

  private:
    Float etaI, etaT; // The index of refraction of the medium through which the incident ray
                      // has traveled, and the index of refraction of the medium into which
                      // the incident ray will be transmitted and refracted.
};

class FresnelConductor : public Fresnel {
  public:
    FresnelConductor(float etaI, float etaT, float k) : etaI(etaI), etaT(etaT), k(k) {}
    Color3f Evaluate(float cosThetaI) const;

  private:
    float etaI, etaT; // The index of refraction of the medium through which the incident ray
                      // has traveled, and the index of refraction of the medium into which
                      // the incident ray will be transmitted and refracted.
    Color3f k;
};
