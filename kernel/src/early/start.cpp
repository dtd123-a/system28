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

extern "C"
{
    void _start()
    {
        BootloaderData bootloader_data = GetBootloaderData();

        limine_framebuffer fb = *bootloader_data.fbData.framebuffers[0];
        uint32_t *fb_ptr = (uint32_t *)fb.address;

        Kernel::CPU::Initialize();
        Kernel::Init::InitializeFlanterm(fb_ptr, fb.width, fb.height, fb.pitch);
        Kernel::Mem::InitializePMM(bootloader_data.memmap);
        Kernel::CPU::SMPSetup(*bootloader_data.smp.cpus);
        
        Kernel::Log(KERNEL_LOG_SUCCESS, "Kernel initialized\n");
        Kernel::Log(KERNEL_LOG_INFO, "Number of CPUs: %d\n", bootloader_data.smp.cpu_count);


        while (true) {
            Kernel::CPU::Halt();
        }
    }
}