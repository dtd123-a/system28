/*
    * cpu.cpp
    * CPU initialization code (architecture specific)
    * Created 02/09/23 DanielH
*/
#include <hal/cpu/gdt.hpp>
#include <hal/cpu/interrupt/idt.hpp>

namespace Kernel::CPU {
    void Initialize()
    {
        Kernel::CPU::GDT::Initialize();
        Kernel::CPU::Interrupts::Initialize();
        /* Other CPU initialization stuff should go here */
    }
}