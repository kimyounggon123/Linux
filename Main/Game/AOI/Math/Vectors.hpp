#ifndef VECTORS_H
#define VECTORS_H


#include <cstdint>
#include <cmath>

template <typename T>
struct IVector2
{
    T x; T y;

    // IVector2(T x, T y): x(x), y(y){}
    // IVector2(const IVector2<T>& other): x(other.x), y(other.y){}
    
    // template <typename U>
    // IVector2(const IVector2<U>& other): 
    //     x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}

    IVector2& operator=(const IVector2<T>& other) { x = other.x; y = other.y; return *this;}
    IVector2 operator+(const IVector2<T>& other) const { return {x + other.x, y + other.y};}
    IVector2 operator-(const IVector2<T>& other) const { return {x - other.x, y - other.y};}

    IVector2 operator*(const T scale) const {return {x * scale, y * scale};}
    IVector2 operator/(const T scale) const {return {x / scale, y / scale};}

    IVector2& operator+=(const IVector2<T>& other) { x += other.x; y += other.y;  return *this; }
    IVector2& operator-=(const IVector2<T>& other) { x -= other.x; y -= other.y; return *this; }
    bool operator==(const IVector2<T>& other) const { return x == other.x && y == other.y; }
};

using Vector2Int = IVector2<int>;
using Vector2Float = IVector2<float>;

namespace Vector2Util
{
    template <typename T>  
    T LengthSquared(const IVector2<T>& v) { return v.x * v.x + v.y * v.y; }  

    template<typename T>
    float Length(const IVector2<T>& v) { return std::sqrt(static_cast<float>(LengthSquared(v))); }

    template<typename T>
    T DistanceSquared(const IVector2<T>& a, const IVector2<T>& b)
    {
        IVector2<T> dvec = a - b;
        return LengthSquared(dvec);
    }

    template<typename T>
    T Distance(const IVector2<T>& a, const IVector2<T>& b) 
    { 
        IVector2<T> dvec = a - b;
        return Length(dvec);
    }

    template<typename T>
    IVector2<float> Normalize(const IVector2<T>& v)
    {
        const float lengthSq = Vector2Util::LengthSquared(v);
        if (lengthSq <= 0.000025f) return { 0.0f, 0.0f };
        const float invLength = 1.0f / std::sqrt(lengthSq);
        IVector2<float> floatVec = v;
        return floatVec * invLength;
    }
};  


#endif