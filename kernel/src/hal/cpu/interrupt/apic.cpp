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
#include <early/bootloader_data.hpp>
#include <hal/cpu/interrupt/idt.hpp>

extern BootloaderData GlobalBootloaderData;

constexpr size_t APIC_TMR_MASKED = 0x10000;
constexpr size_t APIC_TMR_MODE_PERIODIC = (1 << 7);

struct {
    uint32_t LVTRegister;
    uint32_t DivisorRegister;
    uint32_t InitCountRegister;
} GlobalTimerFlags;

enum LAPICRegisters {
    EOI = 0xB0,
    Spurious = 0xF0,
    LVTTimer = 0x320,
    TimerDiv = 0x3E0,
    TimerInitCount = 0x380,
    TimerCurrentCount = 0x390
};

/* Interrupt source override structures */
Kernel::Lib::Vector<InterruptControllerStructure *> GlobalISOStructures;

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
    uintptr_t IOAPICBase = 0;

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

    struct InterruptSourceOverride {
        uint8_t Type; // 2
        uint8_t Length;
        uint8_t Bus;
        uint8_t Source;
        uint32_t GSI;
        uint16_t Flags;
    }__attribute__((packed));

    enum {
        Register_Id = 0x0,
        Register_VerMaxRedir = 0x1, // Version contained in bits 0-7, max amount of redirection entries in bits 16-23
        Register_Priority = 0x2, // Contains the arbitration priority in bits 24-27, rest is reserved.
        Register_RedirEntries = 0x10, // Contains redirection entries until 0x3F
    };

    IOAPIC(void *ptr) {
        base = (IOAPICStructure *)ptr;
        IOAPICBase = base->IOAPICBase;
    }

    void Write(uint32_t reg, uint32_t value) {
        volatile uint32_t *volatile ptr = (uint32_t *)(uintptr_t)GetIOAPICBase();

        ptr[0] = (reg & 0xff);
        ptr[4] = value;
    }

    uint32_t Read(uint32_t reg) {
        volatile uint32_t *volatile ptr = (uint32_t *)(uintptr_t)GetIOAPICBase();

        ptr[0] = (reg & 0xff);
        return ptr[4];
    }

    uintptr_t GetIOAPICBase() {
        return IOAPICBase;
    }

    void SetIOAPICBase(uintptr_t newValue) {
        IOAPICBase = newValue;
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
        for (size_t i = 0; i < GlobalISOStructures.size(); i++) {
            InterruptSourceOverride *iso = (InterruptSourceOverride *)GlobalISOStructures.at(i);

            if (iso->Source == irq) {
                Kernel::Log(KERNEL_LOG_DEBUG, "Using Interrupt Source Override for IRQ %d (now mapped to IRQ %d)\n", irq, iso->GSI);
                irq = iso->GSI;
            }
        }
        WriteRedirectionEntry(irq * 2, redirEntry);
    }
};

using namespace Kernel::ACPI;

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
        if (GlobalMADT->LAPICAddress) {
            LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, TimerInitCount, GlobalTimerFlags.InitCountRegister);
        }
    }

    void InitializeMADT() {
        GlobalMADT = (MADTHeader *)GetACPITable("APIC");
        if (!GlobalMADT) Panic("No APIC present on the system.\n");
    }
    
    Lib::Vector<InterruptControllerStructure *> FindAllInterruptControllers(uint8_t Type) {
        Lib::Vector<InterruptControllerStructure *> vec;

        size_t length = GlobalMADT->StandardHeader.Length - sizeof(MADTHeader) + 2;
        InterruptControllerStructure *current = &GlobalMADT->firstICS;

        while (length > 0) {
            if (current->Type == Type) {
                vec.push_back(current);
            }

            length -= current->Length;
            current = (InterruptControllerStructure *)((uintptr_t)current + current->Length);
        }

        return vec;
    }

    void InitializeIOAPIC() {
        uintptr_t hhdm_base = GlobalBootloaderData.hhdm_response.offset;

        Lib::Vector<InterruptControllerStructure *> ioapic_vec = FindAllInterruptControllers(0x1);
        InterruptControllerStructure *ioapic_structure = ioapic_vec.at(0);
        Kernel::Log(KERNEL_LOG_DEBUG, "Found %d I/O APICs on the system.\n", ioapic_vec.size());
        GlobalISOStructures = FindAllInterruptControllers(0x2);

        if (!ioapic_structure) {
            Panic("No I/O APIC found on system.");
        }
        
        GlobalIOAPIC = new IOAPIC((void *)ioapic_structure);

        uintptr_t ioapic_base = GlobalIOAPIC->GetIOAPICBase();
        VMM::MemoryMap(nullptr, ioapic_base + hhdm_base, ioapic_base, false);
        GlobalIOAPIC->SetIOAPICBase(ioapic_base + hhdm_base);
        
        /* PS/2 Keyboard */
        GlobalIOAPIC->CreateRedirectionEntry(IOAPIC::RedirectionEntry {0x21, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 1);
    }

    void InitializeLAPIC() {
        // Expect LAPIC base to be 4K aligned
        LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, Spurious, LAPICRead((void *)(uintptr_t)GlobalMADT->LAPICAddress, Spurious) | (1 << 8) | 0xff);

        LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, LVTTimer, GlobalTimerFlags.LVTRegister);
        LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, TimerDiv, GlobalTimerFlags.DivisorRegister);
        LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, TimerInitCount, GlobalTimerFlags.InitCountRegister);
    }

    void UninstallLAPICTimer() {
        LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, LVTTimer, APIC_TMR_MASKED);
        LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, TimerDiv, 0);
        LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, TimerInitCount, 0);
    }

    void CalibrateTimer() {
        VMM::MemoryMap(nullptr, (uintptr_t)GlobalMADT->LAPICAddress, (uintptr_t)GlobalMADT->LAPICAddress, false);

        // We are setting up the timer for the BSP, then deleting it after.
        LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, LVTTimer, 0x20);
        LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, TimerDiv, 0x3);
        LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, TimerInitCount, 0xffffffff);
        ACPI::PMTMRSleep(50000); // 50000us = 50ms
        // Stop the timer
        LAPICWrite((void *)(uintptr_t)GlobalMADT->LAPICAddress, LVTTimer, APIC_TMR_MASKED);

        uint32_t calibration = 0xffffffff - LAPICRead((void *)(uintptr_t)GlobalMADT->LAPICAddress, TimerCurrentCount);
        
        GlobalTimerFlags.LVTRegister = 0x20;
        GlobalTimerFlags.DivisorRegister = 0x3;
        GlobalTimerFlags.InitCountRegister = calibration;

        UninstallLAPICTimer();
    }
}
