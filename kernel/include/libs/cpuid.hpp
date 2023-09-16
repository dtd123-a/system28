/*
    * cpuid.hpp
    * __cpuid wrapper
    * Created 15/09/23 DanielH
*/
#pragma once
#include <stdint.h>
#include <cpuid.h>

namespace Kernel {
    /* Wrapper around __cpuid GCC macro to allow it to simply be used with the & operator. */
    static inline void Cpuid(uint32_t level, uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
        __cpuid(level, eax, ebx, ecx, edx);
    }
}