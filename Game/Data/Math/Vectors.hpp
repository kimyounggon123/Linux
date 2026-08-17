#ifndef VECTORS_H
#define VECTORS_H

#include <cstdint>
#include <cmath>

constexpr float PI = 3.1415926535f;
constexpr float HALF_PI = PI * 0.5f;
constexpr float TWO_PI = PI * 2.0f;
constexpr float EPSILON = 1e-6f;

template <typename T>
struct TVector2
{
    T x;
    T y;

    //constexpr TVector2(T x, T y): x(x), y(y) {}
    //TVector2(const TVector2<T>& other): x(other.x), y(other.y) {}

    //template <typename U>
    //TVector2(const TVector2<U>& other): x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}

    //~TVector2(){}

    // opreator overloading
    constexpr TVector2 operator+(const TVector2& rhs) const
    {
        return {x + rhs.x , y + rhs.y}; 
    }
    constexpr TVector2 operator-(const TVector2& rhs) const
    {
        return {x - rhs.x , y - rhs.y}; 
    }
    constexpr TVector2 operator*(T s) const
    {
        return {x * s , y * s}; 
    }

    T GetLengthSq() {return x*x + y*y;}
};
using Vector2Int = TVector2<int>;
using Vector2Float = TVector2<float>;


// namespace Math::SIMD
// {
//     Normalize(v);
//     Dot(a,b);
// }
/*
Normalize(v)
Distance(a,b)
DistanceSq(a,b)
Lerp(a,b,t)
ClampMagnitude(v,maxLen)
Angle(a,b)
Project(a,b)
*/

namespace VectorMath
{
    template<typename T>
    constexpr auto Dot(const TVector2<T>& a, const TVector2<T>& b)
    {
        return a.x * b.x + a.y * b.y;
    }

    template <typename T>
    T DistanceSq(const TVector2<T>& a, const TVector2<T>& b)
    {
        TVector2<T> distance = a - b;
        return distance.GetLengthSq();
    }
};




#endif