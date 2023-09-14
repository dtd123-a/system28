/*
    * smp.cpp
    * Symmetric multi-processing
    * Created 02/09/2023
*/

#include <limine.h>
#include <stddef.h>
#include <hal/cpu/gdt.hpp>
#include <hal/cpu/interrupt/idt.hpp>
#include <hal/cpu/interrupt/lapic.hpp>
#include <hal/cpu.hpp>
#include <terminal/terminal.hpp>

limine_smp_info* smp_info = nullptr;
uint32_t num_cpu = 0; 

namespace Kernel::CPU {
    void SMPSetup(limine_smp_info* smp_info, uint32_t cpu_count) {
        ::smp_info = smp_info;
        num_cpu = cpu_count;
    }

    void CPUJump(unsigned int cpu, void* ptr) {
        if (!cpu) return; // CPU must be > 0
        if (!smp_info) return;

        smp_info[cpu].goto_address = (limine_goto_address)ptr;
    }

    void PerCPUSetup() {
        CPU::GDT::Initialize();
        CPU::Interrupts::Install();
        CPU::InitializeLAPIC();

        while (true) {
            CPU::Halt();
        }
    }

    void SetupAllCPUs() {
        if (!smp_info) return;

        if (num_cpu == 1) {
            Kernel::Log(KERNEL_LOG_DEBUG, "Using single-processor setup.\n");
            // Only LAPIC initialization is required as GDT and IDT are already installed
            // on the BSP.
            CPU::InitializeLAPIC();
            return;
        }

        for (uint32_t i = 1; i < num_cpu; i++) {
            smp_info[i].goto_address = (limine_goto_address)&PerCPUSetup;
        }
    }
}
