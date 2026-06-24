#ifndef UTILS_H
#define UTILS_H

#include <random>
#include <memory>
#include <cstring>
#include <iostream>
// constant Definition
#define BUFFERSIZE 1024

// Type Definition
using BYTE = unsigned char;



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


#endif