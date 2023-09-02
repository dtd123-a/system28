#include <hal/cpu/interrupt/idt.hpp>
#include <hal/cpu.hpp>
#include <terminal/terminal.hpp>

extern "C" void ExceptionHandler();
extern "C" void EH_Stage2(Kernel::CPU::Interrupts::Registers* registers)
{
    Kernel::Log(KERNEL_LOG_FAIL, "Exception interrupt caught in kernel.\n");
    Kernel::Log(KERNEL_LOG_PRINTONLY, "IP = 0x%x, SP = 0x%x\nCS = 0x%x, Flags = 0x%x, SS = 0x%x\nRAX = 0x%x, RBX = 0x%x, RCX = 0x%x, RDI = 0x%x\n",
        registers->rip,
        registers->rsp,
        registers->cs,
        registers->rflags,
        registers->ss,
        registers->rax,
        registers->rbx,
        registers->rcx,
        registers->rdi);


    Kernel::CPU::ClearInterrupts();

    while (true) {
        Kernel::CPU::Halt();    
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
