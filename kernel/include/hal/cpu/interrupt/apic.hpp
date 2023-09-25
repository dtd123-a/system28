
/*
    * apic.hpp
    * Implements the local APIC and its timer.
    * Created 11/09/23 DanielH
*/
#pragma once
#include <hal/acpi.hpp>

struct InterruptControllerStructure {
    //
    // These are the common values in all the Interrupt Controller structure types.
    // Type indicates the type (size of the structure varies for each type), and length is the length of that structure.
    //
    uint8_t Type;
    uint8_t Length;
}__attribute__((packed));

struct MADTFlags {
    bool Dual8259 : 1;
    uint32_t Reserved : 31;
}__attribute__((packed));

struct MADTHeader {
    Kernel::ACPI::SDTHeader StandardHeader;
    uint32_t LAPICAddress;
    MADTFlags Flags;
    InterruptControllerStructure firstICS;
}__attribute__((packed));

namespace Kernel::CPU {
    void InitializeLAPIC();
    void InitializeIOAPIC();
    void InitializeMADT();
    void LAPIC_EOI();
    void TimerReset();
}