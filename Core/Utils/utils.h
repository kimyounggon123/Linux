#ifndef UTILS_H
#define UTILS_H

#include <random>
#include <memory>
#include <cstring>
#include <iostream>


// Heap Allocate & Free
template <typename T>
void SAFE_FREE(T*& ptr)
{
    if (ptr == nullptr) return;
    delete ptr;
    ptr = nullptr;
}

template <typename T>
void SAFE_FREE_ARRAY(T*& ptr)
{
    if (ptr == nullptr) return;
    delete[] ptr;
    ptr = nullptr;
}


template <typename T>
constexpr uint32_t ChangeToUINT(const T& toChange)
{
	return static_cast<uint32_t>(toChange);
}
template <typename T>
constexpr T ChangeToEnumClass(const uint32_t toChange)
{
	return static_cast<T>(toChange);
}

#endif