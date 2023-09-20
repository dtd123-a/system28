/*
    * start.cpp
    * Entry (_start)
    * Created 01/09/23 DanielH
*/
#include <early/bootloader_data.hpp>
#include <terminal/terminal.hpp>
#include <hal/cpu.hpp>
#include <mm/pmm.hpp>
#include <hal/cpu/smp/smp.hpp>
#include <hal/vmm.hpp>
#include <mm/mem.hpp>
#include <hal/acpi.hpp>
#include <hal/cpu/interrupt/apic.hpp>
#include <mm/heap.hpp>
#include <libs/kernel.hpp>

extern "C" void _start()
{
    BootloaderData bootloader_data = GetBootloaderData();

    limine_framebuffer fb = *bootloader_data.fbData.framebuffers[0];
    uint32_t *fb_ptr = (uint32_t *)fb.address;

    Kernel::CPU::Initialize();
    Kernel::Init::InitializeFlanterm(fb_ptr, fb.width, fb.height, fb.pitch);
    Kernel::Mem::InitializePMM(bootloader_data.memmap);

    Kernel::Log(KERNEL_LOG_SUCCESS, "Kernel initializing...\n");
    Kernel::Log(KERNEL_LOG_INFO, "Number of CPUs: %d\n", bootloader_data.smp.cpu_count);
    Kernel::VMM::InitPaging(bootloader_data.memmap, bootloader_data.kernel_addr, bootloader_data.hhdm_response.offset);
    Kernel::ACPI::SetRSDP((uintptr_t)bootloader_data.rsdp_response.address);
    Kernel::CPU::SMPSetup(*bootloader_data.smp.cpus, bootloader_data.smp.cpu_count);
    Kernel::CPU::InitializeMADT();
    Kernel::Mem::InitializeHeap(0x1000 * 10);
    Kernel::CPU::SetupAllCPUs();

    while (true) {
        Kernel::CPU::Halt();
    }
}