/*
    * acpi.cpp
    * ACPI table handling code
    * Created 09/09/23
*/
#include <hal/acpi.hpp>
#include <stdint.h>
#include <terminal/terminal.hpp>
#include <hal/vmm.hpp>
#include <mm/mem.hpp>
#include <libs/string.hpp>
#include <early/bootloader_data.hpp>
#include <libs/kernel.hpp>
#include <hal/cpu/interrupt/apic.hpp>
#include <hal/cpu.hpp>

using Kernel::ACPI::SDTHeader;
using Kernel::ACPI::FADTStructure;

extern BootloaderData GlobalBootloaderData;

/* Holds a pointer to the Fixed ACPI Description Table */
FADTStructure *GlobalFADT = nullptr;

/* RSDP Structure */
struct RSDP {
    char Signature[8];
    uint8_t Checksum;
    char OEMId[6];
    uint8_t Revision;
    uint32_t RSDTPtr;
}__attribute__((packed));

/* Performs the ACPI table checksum, which verifies if it is a valid ACPI table. */
bool SDTChecksum(SDTHeader *tableHeader) {
    uint8_t sum = 0;

    for (uint32_t i = 0; i < tableHeader->Length; i++) {
        sum += ((char *)tableHeader)[i];
    }

    return sum == 0;
}

SDTHeader *MemoryMapACPITable(SDTHeader *tableHeader) {
    /* Get the HHDM base from the bootloader data */
    uintptr_t hhdm_base = GlobalBootloaderData.hhdm_response.offset;

    /* Get the physical address of the table header */
    uintptr_t tableHeaderPhys = (uintptr_t)tableHeader;
    
    /* Now we map only the header, so we know the length of the whole table */
    for (uintptr_t i = ALIGN_DOWN((uintptr_t)tableHeader, 0x1000); i < (uintptr_t)tableHeader + sizeof(SDTHeader); i += 0x1000) {
        Kernel::VMM::MemoryMap(nullptr, hhdm_base + i, i, false);
    }

    /* Now that the know the length, we map the whole table */
    tableHeader = (SDTHeader *)((uintptr_t)tableHeader + hhdm_base);
    for (uintptr_t i = ALIGN_DOWN((uintptr_t)tableHeaderPhys, 0x1000); i < (uintptr_t)tableHeaderPhys + tableHeader->Length; i += 0x1000) {
        Kernel::VMM::MemoryMap(nullptr, hhdm_base + i, i, false);
    }

    /* Return the higher half mapped table pointer */
    return tableHeader;
}

SDTHeader *GlobalRSDT = nullptr;

namespace Kernel::ACPI {
    void SetRSDP(uintptr_t rsdp) {
        RSDP *SystemRSDP = (RSDP *)rsdp;
        /* Set the global RSDT pointer */
        GlobalRSDT = (SDTHeader *)(uintptr_t)SystemRSDP->RSDTPtr;

        /* Map the RSDT into virtual memory */
        GlobalRSDT = MemoryMapACPITable(GlobalRSDT);

        /* Ensure the RSDT passes its checksum */
        if (!SDTChecksum(GlobalRSDT)) {
            Panic("[ACPI] RSDT checksum failed.\n");
        }
    }

    SDTHeader *GetACPITable(const char *Signature) {
        if (!GlobalRSDT) return nullptr;

        /* First, we get the number of table entries in the RSDT. */
        uint32_t entryCount = (GlobalRSDT->Length - sizeof(SDTHeader)) / 4;
        /* Now, we skip over the header and start from the actual table. */
        uintptr_t RSDTTableStart = (uintptr_t)GlobalRSDT + 36; // 36 is the byte offset from the header, where the table entries start.

        /* Loop through each entry in the table */
        for (uint32_t i = 0; i < entryCount; i++) {
            /* Get the actual entry */
            uintptr_t entry = RSDTTableStart + (i * 4);
            uint32_t *entryPtr = (uint32_t *)entry;

            /* Get the header fro the entry*/
            SDTHeader *header = (SDTHeader *)(uintptr_t)*entryPtr;
            /* Map it into virtual memory */
            header = MemoryMapACPITable(header);

            /* Check the signature, and if it matches, return it. */
            if (strncmp(Signature, (const char*)header->Signature, 4) == 0) {
                return header;
            }
        }

        /* The table wasn't found :-( */
        return nullptr;
    }

    /* Fixed rate of the PMT, at 3.579545 MHz */
    constexpr size_t PMT_TMR_RATE = 3579545;

    // Thanks a lot to https://dox.ipxe.org/acpi__timer_8c_source.html and https://wiki.osdev.org/ACPI_Timer
    bool PMTMRSleep(size_t us) {
        /* If PM_TMR_LEN != 4 then the timer shouldn't be used. */
        if (GlobalFADT->PM_TMR_LEN != 4 || !GlobalFADT->PM_TMR_BLK) {
            Log(KERNEL_LOG_FAIL, "PM_TMR delay attemped but failed as timer is unavailable.");
            
            /* Report failiure to the caller. */
            return false;
        }

        size_t count = IO::inl(GlobalFADT->PM_TMR_BLK);
        size_t target = ((us * PMT_TMR_RATE) / 1000000);
        size_t current = 0;

        while (current < target) {
            current = ((IO::inl(GlobalFADT->PM_TMR_BLK) - count) & 0xffffff);
        }

        return true;
    }

    void InitializeACPI(uintptr_t rsdp) {
        /* Set the global RSDP */
        SetRSDP(rsdp);
        
        /* Get the FADT */
        GlobalFADT = (FADTStructure *)GetACPITable("FACP");
        if (!GlobalFADT) Panic("[ACPI] No FADT.");
        if (!SDTChecksum((SDTHeader *)GlobalFADT)) Kernel::Panic("[ACPI] FADT table has failed checksum test.");

        Log(KERNEL_LOG_INFO, "[ACPI] Parsing the Fixed ACPI Description table (FADT)\n");

        switch (GlobalFADT->PreferredPowerManagementProfile) {
            case 1: {
                Log(KERNEL_LOG_INFO, "[ACPI] You are using a desktop\n");
                break;
            }
            case 2: {
                Log(KERNEL_LOG_INFO, "[ACPI] You are using a laptop\n");
                break;
            }
            case 3: {
                Log(KERNEL_LOG_INFO, "[ACPI] You are using a workstation\n");
                break;
            }
            case 4: {
                Log(KERNEL_LOG_INFO, "[ACPI] You are using a server\n");
                break;
            }
            case 5: {
                Log(KERNEL_LOG_INFO, "[ACPI] You are using a SOHO server\n");
                break;
            }
            case 6: {
                Log(KERNEL_LOG_INFO, "[ACPI] You are using an appliance PC\n");
                break;
            }
            case 7: {
                Log(KERNEL_LOG_INFO, "[ACPI] You are using a high-performance server\n");
                break;
            }
            case 8: {
                Log(KERNEL_LOG_INFO, "[ACPI] You are using a tablet\n");
                break;
            }
            default: {
                Log(KERNEL_LOG_INFO, "[ACPI] Unable to detect the device power management profile.\n");
                break;
            }
        }
    }

    /* Performs a warm reboot using ACPI. Returns false if the operation could not be completed. */
    bool PerformACPIReboot() {
        Log(KERNEL_LOG_EVENT, "Attempting to perform ACPI system reset.\n");

        /* Check if the global FADT has been initialized. */
        if (!GlobalFADT) return false;
        /* Check if reboot is supported on the current ACPI revision. */
        if (GlobalFADT->Revision < 2) return false;

        /*
            To perform an ACPI reboot, the 'reset value' must be placed in the 'reset register'.
            The reset value is easily accessible in the FADT, and the reset register can be in memory, regular I/O, or be a PCI-based system.
            Here we only implement the MMIO and I/O methods.
        */
        switch (GlobalFADT->ResetReg.AddressSpace) {
            case GenericAddressStructure::GAS_TYPE_MMIO: { // System memory space
                // Make sure the memory is mapped
                for (uintptr_t i = ALIGN_DOWN(GlobalFADT->ResetReg.Address, 4096); i < GlobalFADT->ResetReg.Address + 1; i += 4096) {
                    VMM::MemoryMap(nullptr, i, i, false);
                }

                uint8_t *reset = (uint8_t *)GlobalFADT->ResetReg.Address;
                *reset = GlobalFADT->ResetValue; // System will reset now!
                
                CPU::ClearInterrupts();
                while (true);

                break;
            }
            case GenericAddressStructure::GAS_TYPE_IO: { // I/O space
                IO::outb(GlobalFADT->ResetReg.Address, GlobalFADT->ResetValue); // System will reset now!

                CPU::ClearInterrupts();
                while (true);

                break;
            }
            case GenericAddressStructure::GAS_TYPE_PCI: {
                Log(KERNEL_LOG_FAIL, "Unimplemented: PCI-based reboot!\n");
                break;
            }
        }

        return false;
    }
}
