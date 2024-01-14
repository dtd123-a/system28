/*
    * terminal.hpp
    * Declares terminal functions
    * Created 01/09/2023 DanielH
*/

#pragma once
#include <stdint.h>

enum KernelLogType {
    KERNEL_LOG_SUCCESS,
    KERNEL_LOG_FAIL,
    KERNEL_LOG_INFO,
    KERNEL_LOG_PRINTONLY,
    KERNEL_LOG_DEBUG,
    KERNEL_LOG_EVENT
};

namespace Kernel {
    namespace Init {
        void InitializeFlanterm(uint32_t *framebuffer, int width, int height, int pitch);
    }

    void Print(const char* string);
    void Log(KernelLogType type, const char *format, ...);
    void PutChar(char c);
}