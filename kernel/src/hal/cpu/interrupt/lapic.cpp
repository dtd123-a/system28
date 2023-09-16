/*
    * lapic.cpp
    * Implements the local APIC and its timer.
    * Created 11/09/23 DanielH
*/
#include <hal/cpu/interrupt/lapic.hpp>
#include <hal/acpi.hpp>
#include <terminal/terminal.hpp>
#include <hal/vmm.hpp>
#include <libs/kernel.hpp>
#include <stddef.h>
#include <libs/cpuid.hpp>

const long int TimerMax = 0x1234567l;

enum LAPICRegisters {
    EOI = 0xB0,
    Spurious = 0xF0,
    LVTTimer = 0x320,
    TimerDiv = 0x3E0,
    TimerInitCount = 0x380,
};

using namespace Kernel::ACPI;

struct MADTHeader {
    SDTHeader StandardHeader;
    uint32_t LAPICAddress;
    uint32_t Flags;
}__attribute__((packed));

MADTHeader *GlobalMADT = nullptr;

void LAPICWrite(void *base, uint32_t reg, uint32_t value) {
    *((uint32_t volatile *)((uintptr_t)base + reg)) = value;
}

uint32_t LAPICRead(void *base, uint32_t reg) {
    return *((uint32_t volatile *)((uintptr_t)base + reg));
}

namespace Kernel::CPU {
    void LAPIC_EOI() {
        if (GlobalMADT->LAPICAddress) LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, EOI, 0);
    }

    void TimerReset() {
        if (GlobalMADT->LAPICAddress) LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, TimerInitCount, TimerMax);
    }

    void InitializeLAPIC() {
        GlobalMADT = (MADTHeader *)GetACPITable("APIC");
        if (!GlobalMADT) Panic("No APIC present on the system.\n");

        // Expect LAPIC base to be 4K aligned
        VMM::MemoryMap(nullptr, (uintptr_t)GlobalMADT->LAPICAddress, (uintptr_t)GlobalMADT->LAPICAddress, false);

        LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, Spurious, LAPICRead((void *)(uintptr_t)GlobalMADT->LAPICAddress, Spurious) | (1 << 8) | 0xff);

        /* Timer Setup */
        // TODO Calibrate the timer
        LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, LVTTimer, 0x20);
        LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, TimerDiv, 0);
        LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, TimerInitCount, TimerMax);
    }
}
