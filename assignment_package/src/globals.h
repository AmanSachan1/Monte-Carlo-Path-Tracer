#pragma once

// Global includes
#include <la.h>
#include <type_traits>
#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <assert.h>
#include <string.h>


// Global constants
static const float ShadowEpsilon = 0.0001f;
static const float RayEpsilon = 0.000005f;
static const float Pi = 3.14159265358979323846;
static const float TwoPi = 6.28318530717958647692;
static const float InvPi = 0.31830988618379067154;
static const float Inv2Pi = 0.15915494309189533577;
static const float Inv4Pi = 0.07957747154594766788;
static const float PiOver2 = 1.57079632679489661923;
static const float PiOver4 = 0.78539816339744830961;
static const float Sqrt2 = 1.41421356237309504880;
static const float OneMinusEpsilon = 0.99999994;

// We're going to create some type aliases to
// give our code a little more context so it's
// easier to interpret at a glance.
// For example, we're going to say that our
// custom types Color3f, Point3f, and Vector3f are
// all aliases of glm::vec3. Since we'll use them in
// different contexts, we want the human-readable distinction.
typedef float Float;
typedef glm::vec3 Color3f;
typedef glm::vec3 Point3f;
typedef glm::vec3 Normal3f;
typedef glm::vec2 Point2f;
typedef glm::ivec2 Point2i;
typedef glm::ivec3 Point3i;
typedef glm::vec3 Vector3f;
typedef glm::vec2 Vector2f;
typedef glm::ivec2 Vector2i;
typedef glm::mat4 Matrix4x4;
typedef glm::mat3 Matrix3x3;

// A cheap way of storing the minimum and maximum bounds of a 2D box
struct Bounds2i : public glm::ivec4
{
    Bounds2i(Point2i min, Point2i max)
        : glm::ivec4(min.x, min.y, max.x, max.y)
    {}

    Point2i Min() const
    {
        return Point2i((*this).x, (*this).y);
    }
    Point2i Max() const
    {
        return Point2i((*this).z, (*this).w);
    }

    Vector2i Diagonal() const
    {
        return this->Max() - this->Min();
    }

    float Area() const
    {
        Vector2i diag = Diagonal();
        return diag.x * diag.y;
    }

    std::vector<Point2i> GetPoints() const
    {
        std::vector<Point2i> result;
        for(int X = Min().x; X < Max().x; X++)
        {
            for(int Y = Min().y; Y < Max().y; Y++)
            {
                result.push_back(Point2i(X, Y));
            }
        }
        return result;
    }
};

//A bunch of useful functions for path tracing, all have self explanatory names

inline bool IsBlack(const Color3f& c)
{
    return (c.r == 0.f && c.g == 0.f && c.b == 0.f);
}

inline float AbsDot(const Vector3f& a, const Vector3f& b)
{
    return glm::abs(glm::dot(a, b));
}

inline bool SameHemisphere(const Vector3f &w, const Vector3f &wp)
{
    return w.z * wp.z > 0;
}

inline float CosTheta(const Vector3f &w) { return w.z; }
inline float Cos2Theta(const Vector3f &w) { return w.z * w.z; }
inline float AbsCosTheta(const Vector3f &w) { return std::abs(w.z); }
inline float Sin2Theta(const Vector3f &w) {
    return std::max((float)0, (float)1 - Cos2Theta(w));
}

inline float SinTheta(const Vector3f &w) { return std::sqrt(Sin2Theta(w)); }

inline float TanTheta(const Vector3f &w) { return SinTheta(w) / CosTheta(w); }

inline float Tan2Theta(const Vector3f &w) {
    return Sin2Theta(w) / Cos2Theta(w);
}

inline float CosPhi(const Vector3f &w)
{
    float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 1 : glm::clamp(w.x / sinTheta, -1.f, 1.f);
}

inline float SinPhi(const Vector3f &w)
{
    float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 0 : glm::clamp(w.y / sinTheta, -1.f, 1.f);
}

inline float Cos2Phi(const Vector3f &w) { return CosPhi(w) * CosPhi(w); }

inline float Sin2Phi(const Vector3f &w) { return SinPhi(w) * SinPhi(w); }

inline bool Refract(const Vector3f &wi, const Normal3f &n, float eta,
                    Vector3f *wt) {
    // Compute cos theta using Snell's law
    float cosThetaI = glm::dot(n, wi);
    float sin2ThetaI = std::max(float(0), float(1 - cosThetaI * cosThetaI));
    float sin2ThetaT = eta * eta * sin2ThetaI;

    // Handle total internal reflection for transmission
    if (sin2ThetaT >= 1) return false;
    float cosThetaT = std::sqrt(1 - sin2ThetaT);
    *wt = eta * -wi + (eta * cosThetaI - cosThetaT) * Vector3f(n);
    return true;
}

inline Normal3f Faceforward(const Normal3f &n, const Vector3f &v) {
    return (glm::dot(n, v) < 0.f) ? -n : n;
}

// Create a set of axes to form the basis of a coordinate system
// given a single vector v1.
inline void CoordinateSystem(const Vector3f& v1, Vector3f* v2, Vector3f* v3)
{
    if (std::abs(v1.x) > std::abs(v1.y))
            *v2 = Vector3f(-v1.z, 0, v1.x) / std::sqrt(v1.x * v1.x + v1.z * v1.z);
        else
            *v2 = Vector3f(0, v1.z, -v1.y) / std::sqrt(v1.y * v1.y + v1.z * v1.z);
        *v3 = glm::cross(v1, *v2);
}
