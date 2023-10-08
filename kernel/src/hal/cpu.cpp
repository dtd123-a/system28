/*
    * cpu.cpp
    * CPU initialization code (architecture specific)
    * Created 02/09/23 DanielH
*/
#include <hal/cpu/gdt.hpp>
#include <hal/cpu/interrupt/idt.hpp>
#include <hal/cpu.hpp>

bool SystemCrashFlag = false;

namespace Kernel::CPU {
    void Initialize() {
        Kernel::CPU::GDT::Initialize();
        Kernel::CPU::Interrupts::Initialize();
        Kernel::CPU::Interrupts::Install();
    }

    void SetCrash() {
        SystemCrashFlag = true;
    }
}

/* For C code */
extern "C" void CPUPause() {
    Kernel::CPU::Pause();
}
