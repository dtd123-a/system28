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

FADTStructure *GlobalFADT = nullptr;

struct RSDP {
    char Signature[8];
    uint8_t Checksum;
    char OEMId[6];
    uint8_t Revision;
    uint32_t RSDTPtr;
}__attribute__((packed));

bool SDTChecksum(SDTHeader *tableHeader) {
    uint8_t sum = 0;

    for (uint32_t i = 0; i < tableHeader->Length; i++) {
        sum += ((char *)tableHeader)[i];
    }

    return sum == 0;
}

SDTHeader *MemoryMapACPITable(SDTHeader *tableHeader) {
    uintptr_t hhdm_base = GlobalBootloaderData.hhdm_response.offset;

    // First map the header so we know the size
    uintptr_t tableHeaderPhys = (uintptr_t)tableHeader;

    for (uintptr_t i = ALIGN_DOWN((uintptr_t)tableHeader, 0x1000); i < (uintptr_t)tableHeader + sizeof(SDTHeader); i += 0x1000) {
        Kernel::VMM::MemoryMap(nullptr, hhdm_base + i, i, false);
    }
    tableHeader = (SDTHeader *)((uintptr_t)tableHeader + hhdm_base);
    // Then from the start of the table let's map the whole length of it
    for (uintptr_t i = ALIGN_DOWN((uintptr_t)tableHeaderPhys, 0x1000); i < (uintptr_t)tableHeaderPhys + tableHeader->Length; i += 0x1000) {
        Kernel::VMM::MemoryMap(nullptr, hhdm_base + i, i, false);
    }

    return tableHeader;
}

SDTHeader *GlobalRSDT = nullptr;
bool RSDTPassedChecksum = true;

namespace Kernel::ACPI {
    void SetRSDP(uintptr_t rsdp) {
        RSDP *SystemRSDP = (RSDP *)rsdp;
        GlobalRSDT = (SDTHeader *)(uintptr_t)SystemRSDP->RSDTPtr;

        // Map the RSDT
        GlobalRSDT = MemoryMapACPITable(GlobalRSDT);

        if (!SDTChecksum(GlobalRSDT)) {
            Kernel::Log(KERNEL_LOG_FAIL, "Warning: Checksum failed on RSDT! ACPI features will not work.\n");
            RSDTPassedChecksum = false;
        }
    }

    SDTHeader *GetACPITable(const char *Signature) {
        if (!GlobalRSDT || !RSDTPassedChecksum) return nullptr;

        uint32_t entryCount = (GlobalRSDT->Length - sizeof(SDTHeader)) / 4;
        uintptr_t RSDTTableStart = (uintptr_t)GlobalRSDT + 36; // 36 is the byte offset from the header, where the table entries start.

        for (uint32_t i = 0; i < entryCount; i++) {
            uintptr_t entry = RSDTTableStart + (i * 4);
            uint32_t *entryPtr = (uint32_t *)entry;
            SDTHeader *header = (SDTHeader *)(uintptr_t)*entryPtr;
            header = MemoryMapACPITable(header);

            if (strncmp(Signature, (const char*)header->Signature, 4) == 0) {
                return header;
            }
        }

        return nullptr;
    }
    
    void InitializeACPI() {
        GlobalFADT = (FADTStructure *)GetACPITable("FACP");
        if (!GlobalFADT) return;
        if (!SDTChecksum((SDTHeader *)GlobalFADT)) Kernel::Panic("ACPI table has failed checksum test.");

        Kernel::Log(KERNEL_LOG_INFO, "ACPI: Parsing ACPI \"FADT\" table\n");

        switch (GlobalFADT->PreferredPowerManagementProfile) {
            case 1: {
                Kernel::Log(KERNEL_LOG_INFO, "ACPI: You are using a desktop\n");
                break;
            }
            case 2: {
                Kernel::Log(KERNEL_LOG_INFO, "ACPI: You are using a laptop\n");
                break;
            }
            case 3: {
                Kernel::Log(KERNEL_LOG_INFO, "ACPI: You are using a workstation\n");
                break;
            }
            case 4: {
                Kernel::Log(KERNEL_LOG_INFO, "ACPI: You are using a server\n");
                break;
            }
            case 5: {
                Kernel::Log(KERNEL_LOG_INFO, "ACPI: You are using a SOHO server\n");
                break;
            }
            case 6: {
                Kernel::Log(KERNEL_LOG_INFO, "ACPI: You are using an appliance PC\n");
                break;
            }
            case 7: {
                Kernel::Log(KERNEL_LOG_INFO, "ACPI: You are using a high-performance server\n");
                break;
            }
            case 8: {
                Kernel::Log(KERNEL_LOG_INFO, "ACPI: You are using a tablet\n");
                break;
            }
            default: {
                Kernel::Log(KERNEL_LOG_INFO, "ACPI: Device type is undefined, or you are using an emulator.\n");
                break;
            }
        }
    }

    /* Performs a warm reboot using ACPI. Returns false if the operation could not be completed. */
    bool PerformACPIReboot() {
        Kernel::Log(KERNEL_LOG_EVENT, "Attempting to perform ACPI system reset.\n");

        if (!GlobalFADT) return false;
        if (GlobalFADT->Revision < 2) return false;

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
            /* TODO Implement the PCI-based reboot */
        }

        return false;
    }
}
