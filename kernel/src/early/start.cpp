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
#include <hal/acpi.hpp>
#include <mm/heap.hpp>
#include <logo.h>
#include <obj/mod.hpp>

LIMINE_BASE_REVISION(1)

/* Global kernel boot loader information state */
BootloaderData GlobalBootloaderData;

using namespace Kernel;

/* The procedure called by the boot loader */
extern "C" void _start()
{
    GlobalBootloaderData = GetBootloaderData();

    /* Sets up the Global Descriptor Table & Exception Handling for the BSP. */
    CPU::Initialize();

    /* Set up the terminal emulator */
    limine_framebuffer fb = *GlobalBootloaderData.fbData->framebuffers[0];
    Init::InitializeFlanterm((uint32_t *)fb.address, fb.width, fb.height, fb.pitch);

    /* Print the System/28 splash screen */
    Print(System28ASCII());

    /* Initialize the Physical Memory Allocator */
    Mem::InitializePMM(*GlobalBootloaderData.memmap);

    /* Initialize virtual memory paging */
    VMM::InitPaging(*GlobalBootloaderData.memmap, *GlobalBootloaderData.kernel_addr);

    /* Switch to the kernel's page tables */
    VMM::LoadKernelCR3();

    /* Initialize ACPI */
    ACPI::InitializeACPI((uintptr_t)GlobalBootloaderData.rsdp_response->address);

    /* Set up the heap manager */
    Mem::InitializeHeap(0x1000 * 10);

    /* Set up the rest of the CPU cores */
    CPU::SetupAllCPUs();

    /* Handle any modules passed into the kernel */
    Obj::HandleModuleObjects(GlobalBootloaderData.module_response);

    while (true) {
        CPU::Halt();
    }
}
