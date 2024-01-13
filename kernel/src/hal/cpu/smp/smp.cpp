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
#include <hal/vmm.hpp>

limine_smp_info *SMPData = nullptr;
size_t CoreCount = 0;
size_t CoresInitialized = 1; // 1 == BSP

extern BootloaderData GlobalBootloaderData;

namespace Kernel::CPU {
    void CPUJump(uint32_t Core, void* Target) {
        if (!Core) return; // CPU must be > 0
        if (!SMPData) return;

        // An atomic write of a memory address to the "goto_address" field causes the CPU to jump to that address. (Limine spec.)
        SMPData[Core].goto_address = (limine_goto_address)Target;
    }

    /* Per-CPU setup: The initialization code run on each CPU as they are brought online */
    void CPUStartPayload(limine_smp_info *CPUData) {
        CPU::GDT::Load();
        CPU::Interrupts::Install();
        CPU::InitializeLAPIC();
        VMM::LoadKernelCR3();

        Log(KERNEL_LOG_SUCCESS, "[SMP Stage 3] CPU %d is online and waiting for interrupts.\n", CPUData->processor_id + 1);

        CoresInitialized++;

        while (true) {
            CPU::Halt();
        }
    }

    void SetupAllCPUs() {
        /* Set up our global SMP state */
        if (!GlobalBootloaderData.smp) Panic("No SMP data provided by the bootloader, please ensure that the IO APIC is available.\n");

        SMPData = *GlobalBootloaderData.smp->cpus;
        CoreCount = GlobalBootloaderData.smp->cpu_count;

        /* Detects if there is a MADT, sets up MADT state and panics if there is no MADT */
        CPU::InitializeMADT();

        Log(KERNEL_LOG_SUCCESS, "[SMP Stage 1] Calibrating Local APIC timer\n");

        /* Calibrate the APIC timer*/
        if (!CalibrateTimer()) {
            Panic("[SMP Stage 1] Was not able to calibrate the APIC timer.\n");
        }

        Log(KERNEL_LOG_SUCCESS, "[SMP Stage 1] Calibration reported as successful.\n");

        /* Disable interrupts for now */
        ClearInterrupts();

        Log(KERNEL_LOG_SUCCESS, "[SMP Stage 2] Initializing the APIC\n");
        /* Set up the Local APIC on the current processor */
        CPU::InitializeLAPIC();
        
        if (CoreCount == 1) {
            /* There's only one CPU, skip SMP setup. */
            Log(KERNEL_LOG_INFO, "[SMP Stage 3] Using single processor setup.\n");
        } else {
            /* This is a multiprocessor system */
            Log(KERNEL_LOG_SUCCESS, "[SMP Stage 3] Detected multiple CPUs, initializing them now...\n");

            for (size_t i = 1; i < CoreCount; i++) {
                CPUJump(i, (void *)CPUStartPayload);
            }
        }

        SetInterrupts(); // Interrupts are enabled for the BSP.
        CPU::InitializeIOAPIC();
        
        /*  Acknowledge and discard the last keyboard IRQ 
            in case the user hit a key before the IO APIC 
            was set up, which would result in keyboard 
            interrupts "not working".
        */
        IO::inb(0x60);


        while (CoresInitialized < CoreCount); // Wait for all CPUs to be initialized before continuing kernel initialization.
    }
}
