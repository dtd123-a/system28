#pragma once

#define ALIGN_UP(value, boundary) (((value) + ((boundary) - 1)) / (boundary) * (boundary))
#define ALIGN_DOWN(value, boundary)  ((value) & (~((boundary) - 1)))

extern "C" void* memcpy(char* destination, char* source, int n);
extern "C" void* memset(char* destination, int val, int n);