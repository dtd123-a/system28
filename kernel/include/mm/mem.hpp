/*
    * mem.hpp
    * Memory function declarations
    * Created 02/09/2023
*/

#pragma once

#define ALIGN_UP(value, boundary) (((value) + ((boundary) - 1)) / (boundary) * (boundary))
#define ALIGN_DOWN(value, boundary)  ((value) & (~((boundary) - 1)))

extern "C" void* memcpy(void* destination, void* source, unsigned long n);
extern "C" void* memset(void* destination, int val, unsigned long n);
