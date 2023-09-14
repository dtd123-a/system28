/*
    * kernel.hpp
    * Implements some kernel panic functions.
    * Created 12/09/23 DanielH
*/
#pragma once
#include <hal/cpu/interrupt/idt.hpp>

enum KPanicType {
    PANIC_CPU_EXCEPTION = 0,
    PANIC_SOFT_ERROR = 1
};

namespace Kernel {
    void Panic(const char *error);
    void PanicFromException(CPU::Interrupts::CInterruptRegisters *registers, int error_code);
}