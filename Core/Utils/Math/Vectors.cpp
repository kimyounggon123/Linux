#include "Vectors.hpp"

template<>
bool IVector2<float>::operator==(const IVector2<float>& other) const
{
    return std::abs(x - other.x) < 0.05f && std::abs(y - other.y) < 0.05f;
}