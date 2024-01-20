/*
    * cpu.cpp
    * CPU initialization code (architecture specific)
    * Created 02/09/2023 DanielH
*/

#include <hal/cpu/gdt.hpp>
#include <hal/cpu/interrupt/idt.hpp>
#include <hal/cpu.hpp>

namespace Kernel::CPU {
    void Initialize() {
        GDT::Load();
        Interrupts::Initialize();
        Interrupts::Install();
    }
}

/* For C code */
extern "C" void CPUPause() {
    Kernel::CPU::Pause();
}
