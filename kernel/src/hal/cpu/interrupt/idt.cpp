#include <hal/cpu/interrupt/idt.hpp>
#include <hal/cpu.hpp>
#include <terminal/terminal.hpp>

using namespace Kernel::CPU;

__attribute__((interrupt)) void ExceptionHandler(Interrupts::CInterruptRegisters* registers)
{
    Kernel::Log(KERNEL_LOG_FAIL, "** STOP !! **\n");
    Kernel::Log(KERNEL_LOG_FAIL, "CPU Exception in kernel!!\n");
    Kernel::Log(KERNEL_LOG_FAIL, "{IP=0x%x, CS=0x%x, Flags=0x%x, SP=0x%x, SS=0x%x}\n",
    registers->ip,
    registers->cs,
    registers->flags,
    registers->sp,
    registers->ss);

    while (true) {
        Halt();    
    }
}

namespace Kernel::CPU::Interrupts {
    IDTEntry IDT[256];
    IDTR IDTPtr;

    void CreateIDTEntry(int interrupt, void* handler, uint8_t gate_type)
    {
        IDT[interrupt].Offset0 = (uint16_t)((uint64_t)handler & 0x000000000000ffff);
        IDT[interrupt].Offset1 = (uint16_t)(((uint64_t)handler & 0x00000000ffff0000) >> 16);      
        IDT[interrupt].Offset2 = (uint32_t)(((uint64_t)handler & 0xffffffff00000000) >> 32);
        IDT[interrupt].TypesAttrib = gate_type;
        IDT[interrupt].Ist = 0; // No IST
        IDT[interrupt].Selector = 0x08; // Kernel code segment
        IDT[interrupt].Reserved = 0;
    }

    void Initialize()
    {
        /* First, setup exception handlers */
        for (int i = 0; i < 0x1f; i++) {
            CreateIDTEntry(i, (void*)&ExceptionHandler, 0x8F);
        }

        /* Now we setup the IDTR */
        IDTPtr.Limit = 0xfff;
        IDTPtr.Addr = (uint64_t)&IDT;

        /* Load IDT */
        asm ("lidt %0" : : "m" (IDTPtr));
    }
}
