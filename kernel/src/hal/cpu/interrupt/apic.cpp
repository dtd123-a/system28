/*
    * lapic.cpp
    * Implements the local APIC and its timer.
    * Created 11/09/23 DanielH
*/
#include <hal/cpu/interrupt/apic.hpp>
#include <hal/acpi.hpp>
#include <terminal/terminal.hpp>
#include <hal/vmm.hpp>
#include <libs/kernel.hpp>
#include <mm/mem.hpp>
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

struct InterruptControllerStructure {
    //
    // These are the common values in all the Interrupt Controller structure types.
    // Type indicates the type (size of the structure varies for each type), and length is the length of that structure.
    //
    uint8_t Type;
    uint8_t Length;
}__attribute__((packed));

// 
// IOAPIC class
// Contains definitions and methods to interact with the IOAPIC
//
class IOAPIC {
    struct IOAPICStructure {
        //
        // https://uefi.org/specs/ACPI/6.5/05_ACPI_Software_Programming_Model.html#io-apic-structure
        //
        uint8_t Type;
        uint8_t Length;
        uint8_t IOAPICId;
        uint8_t Reserved;
        uint32_t IOAPICBase;
        uint32_t GSIBase;
    }__attribute__((packed));

    IOAPICStructure *base; // I/O APIC base

public:
    struct RedirectionEntry {
        uint8_t Vector; // The IDT interrupt vector
        uint8_t DeliveryMode : 3;
        uint8_t DestinationMode : 1;
        uint8_t Busy : 1;
        uint8_t Polarity : 1;
        uint8_t LTIStatus : 1;
        uint8_t TriggerMode : 1;
        uint8_t InterruptMask : 1;
        uint64_t Reserved : 39;
        uint8_t Destination;
    }__attribute__((packed));

    enum {
        Register_Id = 0x0,
        Register_VerMaxRedir = 0x1, // Version contained in bits 0-7, max amount of redirection entries in bits 16-23
        Register_Priority = 0x2, // Contains the arbitration priority in bits 24-27, rest is reserved.
        Register_RedirEntries = 0x10, // Contains redirection entries until 0x3F
    };

    IOAPIC(void *ptr) {
        base = (IOAPICStructure *)ptr;
    }

    void Write(uint32_t reg, uint32_t value) {
        uint32_t volatile *ptr = (uint32_t volatile *)(uintptr_t)base->IOAPICBase;

        ptr[0] = (reg & 0xff);
        ptr[4] = value;
    }

    uint32_t Read(uint32_t reg) {
        uint32_t volatile *ptr = (uint32_t volatile *)(uintptr_t)base->IOAPICBase;

        ptr[0] = (reg & 0xff);
        return ptr[4];
    }

    uintptr_t GetIOAPICBase() {
        return (uintptr_t)base->IOAPICBase;
    }

    RedirectionEntry ReadRedirectionEntry(uint32_t entry) {
        if ((entry % 2) != 0) return RedirectionEntry {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        //
        // I/O APIC redirection entries are split into two 32 bit values to make the 64-bit structure.
        //
        uint32_t val0 = Read(Register_RedirEntries + entry);
        uint32_t val1 = Read(Register_RedirEntries + (entry + 1));

        uint64_t val = ((uint64_t)val1 << 32) | ((uint64_t)val0);

        return *(RedirectionEntry*)&val;
    }

    void WriteRedirectionEntry(uint32_t entry, RedirectionEntry redirEntry) {
        if ((entry % 2) != 0) return;

        uint64_t value = *(uint64_t*)&redirEntry;
        uint32_t val0 = (uint32_t)value;
        uint32_t val1 = value >> 32;

        Write(Register_RedirEntries + entry, val0);
        Write(Register_RedirEntries + (entry + 1), val1);
    }

    void CreateRedirectionEntry(RedirectionEntry redirEntry, int irq) {
        WriteRedirectionEntry(irq * 2, redirEntry);
    }
};

using namespace Kernel::ACPI;

struct MADTHeader {
    SDTHeader StandardHeader;
    uint32_t LAPICAddress;
    uint32_t Flags;
    InterruptControllerStructure firstICS;
}__attribute__((packed));

MADTHeader *GlobalMADT = nullptr;
IOAPIC *GlobalIOAPIC = nullptr;

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

    void InitializeMADT() {
        GlobalMADT = (MADTHeader *)GetACPITable("APIC");
        if (!GlobalMADT) Panic("No APIC present on the system.\n");
    }

    InterruptControllerStructure *FindInterruptController(uint8_t Type) {
        size_t length = GlobalMADT->StandardHeader.Length - sizeof(MADTHeader) + 2;
        InterruptControllerStructure *current = &GlobalMADT->firstICS;

        while (length > 0) {
            if (current->Type == Type) {
                return current;
            }

            length -= current->Length;
            current = (InterruptControllerStructure *)((uintptr_t)current + current->Length);
        }

        return nullptr;
    }

    void InitializeIOAPIC() {
        InterruptControllerStructure *ioapic_structure = FindInterruptController(0x1);
        GlobalIOAPIC = new IOAPIC((void *)ioapic_structure);

        for (uintptr_t i = ALIGN_DOWN(GlobalIOAPIC->GetIOAPICBase(), 4096); i < GlobalIOAPIC->GetIOAPICBase() + 1; i += 4096) {
            VMM::MemoryMap(nullptr, i, i, false);
        }
        
        //
        //  * Keyboard IRQ example
        //  * GlobalIOAPIC->CreateRedirectionEntry(IOAPIC::RedirectionEntry {0x21, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 1);
        //
    }

    void InitializeLAPIC() {
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
