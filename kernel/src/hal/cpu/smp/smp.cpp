/*
    * smp.cpp
    * Symmetric multi-processing
    * Created 02/09/2023
*/

#include <limine.h>
#include <stddef.h>
#include <hal/cpu/gdt.hpp>
#include <hal/cpu/interrupt/idt.hpp>
#include <hal/cpu/interrupt/apic.hpp>
#include <hal/cpu.hpp>
#include <terminal/terminal.hpp>
#include <libs/kernel.hpp>
#include <hal/acpi.hpp>
#include <early/bootloader_data.hpp>
#include <hal/cpu/interrupt/apic.hpp>

limine_smp_info *SMPData = nullptr;
uint32_t CPUCoreCount = 0;
extern BootloaderData GlobalBootloaderData;

namespace Kernel::CPU {
    void CPUJump(uint32_t Core, void* Target) {
        if (!Core) return; // CPU must be > 0
        if (!SMPData) return;

        // An atomic write of a memory address to the "goto_address" field causes the CPU to jump to that address. (Limine spec.)
        SMPData[Core].goto_address = (limine_goto_address)Target;
    }

    /* Per-CPU setup: The initialization code run on each CPU as they are brought online */
    void PerCPUSetup(limine_smp_info *CPUData) {
        CPU::GDT::Load();
        CPU::Interrupts::Install();
        CPU::InitializeLAPIC();

        Log(KERNEL_LOG_SUCCESS, "[SMP Stage 3] CPU %d is online and waiting for interrupts.\n", CPUData->processor_id + 1);

        while (true) {
            CPU::Halt();
        }
    }

    void SetupAllCPUs() {
        /* Set up our global SMP state */
        SMPData = *GlobalBootloaderData.smp.cpus;
        CPUCoreCount = GlobalBootloaderData.smp.cpu_count;

        /* Detects if there is a MADT, sets up MADT state and panics if there is no MADT */
        CPU::InitializeMADT();

        Log(KERNEL_LOG_SUCCESS, "[SMP Stage 1] Calibrating Local APIC timer\n");

        /* Calibrate the APIC timer*/
        if (!CalibrateTimer()) {
            Panic("[SMP Stage 1] Was not able to calibrate the APIC timer.\n");
        } else Log(KERNEL_LOG_SUCCESS, "[SMP Stage 1] Calibration reported as successful.\n");
        
        ClearInterrupts();

        Log(KERNEL_LOG_SUCCESS, "[SMP Stage 2] Initializing the APIC\n");
        /* Set up the Local APIC on the first processor (which we are currently running on) */
        CPU::InitializeLAPIC();
        
        /* If there is more than one CPU, set up each one */
        if (CPUCoreCount > 1) {
            Log(KERNEL_LOG_SUCCESS, "[SMP Stage 3] Detected multiple CPUs, instructing them now...\n");

            for (uint32_t i = 1; i < CPUCoreCount; i++) {
                CPUJump(i, (void *)&PerCPUSetup);
            }
        } else {
            /* Otherwise, we don't do anything and use the first and only CPU. */
            Log(KERNEL_LOG_INFO, "[SMP Stage 3] Using single processor setup.\n");
        }

        SetInterrupts(); // Interrupts are enabled for the BSP.
        CPU::InitializeIOAPIC(); // Setup I/O APIC globally.
        
        /*  Acknowledge and discard the last keyboard IRQ 
            in case the user hit a key before the IO APIC 
            was set up, which would result in keyboard 
            interrupts "not working".
        */
        IO::inb(0x60);
    }
}
