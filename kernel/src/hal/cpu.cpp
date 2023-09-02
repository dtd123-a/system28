/*
    * cpu.cpp
    * CPU initialization code (architecture specific)
    * Created 02/09/23 DanielH
*/
#include <hal/cpu/gdt.hpp>

namespace Kernel::CPU {
    void Initialize()
    {
        Kernel::CPU::GDT::Initialize();
        /* Other CPU initialization stuff should go here */
    }
}