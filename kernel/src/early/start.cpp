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

BootloaderData GlobalBootloaderData;
using namespace Kernel;

extern "C" void _start()
{
    GlobalBootloaderData = GetBootloaderData();
    
    CPU::Initialize();
    limine_framebuffer fb = *GlobalBootloaderData.fbData.framebuffers[0];
    Init::InitializeFlanterm((uint32_t *)fb.address, fb.width, fb.height, fb.pitch);
    Mem::InitializePMM(GlobalBootloaderData.memmap);
    Print(System28ASCII());
    VMM::InitPaging(GlobalBootloaderData.memmap, GlobalBootloaderData.kernel_addr);
    ACPI::InitializeACPI((uintptr_t)GlobalBootloaderData.rsdp_response.address);
    Mem::InitializeHeap(0x1000 * 10);
    CPU::SetupAllCPUs();
    Obj::HandleModuleObjects(GlobalBootloaderData.module_response);

    while (true) {
        CPU::Halt();
    }
}