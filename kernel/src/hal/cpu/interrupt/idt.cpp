#include <hal/cpu/interrupt/idt.hpp>
#include <hal/cpu.hpp>
#include <terminal/terminal.hpp>
#include <hal/cpu/interrupt/apic.hpp>
#include <libs/kernel.hpp>
#include <hal/cpu/smp/smp.hpp>

using namespace Kernel::CPU;

extern bool SystemCrashFlag;

extern "C" void DisablePIC();

__attribute__((interrupt)) void ExceptionHandler(Interrupts::CInterruptRegisters *registers) {
    Kernel::CPU::ClearInterrupts();
    Kernel::PanicFromException(registers, 0);

    while (true) {
        Halt();    
    }
}

/* Some exceptions push an error code onto the stack, this one handles those. */
__attribute__((interrupt)) void ExceptionHandler2(Interrupts::CInterruptRegisters *registers, uintptr_t error_code) {
    Kernel::CPU::ClearInterrupts();
    Kernel::PanicFromException(registers, error_code);

    while (true) {
        Halt();    
    }
}

__attribute__((interrupt)) void TimerInterrupt(Interrupts::CInterruptRegisters *) {
    TimerReset();
    LAPIC_EOI();
}

constexpr uint8_t DeleteScancode = 0x53;
__attribute__((interrupt)) void KeyboardInterrupt(Interrupts::CInterruptRegisters *) {
    uint8_t scan = Kernel::IO::inb(0x60);

    if (scan == DeleteScancode) {
        if (!Kernel::ACPI::PerformACPIReboot()) Kernel::Log(KERNEL_LOG_FAIL, "Unable to perform ACPI reboot.\n");
    }

    LAPIC_EOI();
}

namespace Kernel::CPU::Interrupts {
    IDTEntry IDT[256];
    IDTR IDTPtr;

    void CreateIDTEntry(int interrupt, void *handler, uint8_t gate_type)
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
            if (i == 0x8 ||
            i == 0xA ||
            i == 0xB ||
            i == 0xC ||
            i == 0xE ||
            i == 0x11 ||
            i == 0x15 ||
            i == 0x1D ||
            i == 0x1E
            ) {
                // Bug? On software-triggered interrupts the error code isn't pushed 
               CreateIDTEntry(i, (void*)&ExceptionHandler2, 0x8F);
            } else {
                CreateIDTEntry(i, (void*)&ExceptionHandler, 0x8F);
            }
        }

        CreateIDTEntry(0x20, (void *)TimerInterrupt, 0x8E);
        CreateIDTEntry(0x21, (void *)KeyboardInterrupt, 0x8E);

        /* Now we setup the IDTR */
        IDTPtr.Limit = 0xfff;
        IDTPtr.Addr = (uint64_t)&IDT;

        DisablePIC();
        // TODO we also need to remap the PIC so if it gives spurious interrupts in the future
        //      it doesn't clash with exception interrupts.
    }

    void Install() {
        /* Load IDT */
        asm ("lidt %0" : : "m" (IDTPtr));
        asm ("sti");
    }
}