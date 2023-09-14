/*
    * kernel.cpp
    * Implements some kernel panic functions.
    * Created 12/09/23 DanielH
*/
#include <terminal/terminal.hpp>
#include <libs/kernel.hpp>
#include <hal/cpu/interrupt/idt.hpp>
#include <hal/cpu.hpp>

namespace Kernel {
    void Panic(const char *error) {
        Kernel::Log(KERNEL_LOG_FAIL, "** STOP !! **\n");
        Kernel::Log(KERNEL_LOG_FAIL, "A serious error has occured in the kernel component.\n");
        Kernel::Log(KERNEL_LOG_FAIL, "Error string = %s\n", error);

        while (true) {
            CPU::Halt();
        }
    }

    void PanicFromException(CPU::Interrupts::CInterruptRegisters *registers, int error_code) {
        Kernel::Log(KERNEL_LOG_FAIL, "** STOP !! **\n");
        Kernel::Log(KERNEL_LOG_FAIL, "CPU Exception in kernel!!\n");
        Kernel::Log(KERNEL_LOG_FAIL, "{IP=0x%x, CS=0x%x, Flags=0x%x, SP=0x%x, SS=0x%x, ERR_CODE=0x%x}\n",
        registers->ip,
        registers->cs,
        registers->flags,
        registers->sp,
        registers->ss,
        error_code);

        while (true) {
            CPU::Halt();
        }
    }
}