/*
    * kernel.cpp
    * Implements kernel utility libraries
    * Created 12/09/23 DanielH
*/
#include <terminal/terminal.hpp>
#include <libs/kernel.hpp>
#include <hal/cpu/interrupt/idt.hpp>
#include <hal/cpu.hpp>
#include <hal/spinlock.hpp>

void *operator new(size_t size) {
    return Kernel::Mem::Allocate(size);
}

void operator delete(void *object) {
    Kernel::Mem::Free(object);
}

void operator delete(void *object, size_t) {
    Kernel::Mem::Free(object);
}

void operator delete[](void *object) {
    Kernel::Mem::Free(object);
}

void operator delete[](void *object, size_t) {
    Kernel::Mem::Free(object);
}

namespace Kernel {
    SPINLOCK_CREATE(PanicLock);
    __attribute__((noreturn)) void Panic(const char *error) {
        SpinlockAquire(&PanicLock);
        Kernel::Log(KERNEL_LOG_FAIL, "** STOP !! **\n");
        Kernel::Log(KERNEL_LOG_FAIL, "A serious error has occured in the kernel component.\n");
        Kernel::Log(KERNEL_LOG_FAIL, "Error string = %s\n", error);

        SpinlockRelease(&PanicLock);

        while (true) {
            CPU::Halt();
        }
    }

    SPINLOCK_CREATE(PanicLock2);
    __attribute__((noreturn)) void PanicFromException(CPU::Interrupts::CInterruptRegisters *registers, int error_code) {
        SpinlockAquire(&PanicLock2);

        Kernel::Log(KERNEL_LOG_FAIL, "** STOP !! **\n");
        Kernel::Log(KERNEL_LOG_FAIL, "CPU Exception in kernel!!\n");
        Kernel::Log(KERNEL_LOG_FAIL, "{IP=0x%x, CS=0x%x, Flags=0x%x, SP=0x%x, SS=0x%x, ERR_CODE=0x%x}\n",
        registers->ip,
        registers->cs,
        registers->flags,
        registers->sp,
        registers->ss,
        error_code);

        SpinlockRelease(&PanicLock2);

        while (true) {
            CPU::Halt();
        }
    }
}