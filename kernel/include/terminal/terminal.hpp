/*
    * terminal.hpp
    * Declares terminal functions
    * Created 01/09/2023 DanielH
*/

#pragma once
#include <stdint.h>
#include <hal/debug/serial.hpp>

using namespace Kernel;

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
        void InitializeFlanterm(uint32_t *framebuffer, int width, int height, int pitch, int red_mask_size, int red_mask_shift, int green_mask_size, int green_mask_shift, int blue_mask_size, int blue_mask_shift);
        void SetSerialOutputPort(Debug::SerialPort *port);
    }

    void Print(const char* string);
    void Log(KernelLogType type, const char *format, ...);
    void PutChar(char c);
}