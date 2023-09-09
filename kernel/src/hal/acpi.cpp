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

using Kernel::ACPI::SDTHeader;

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

void MemoryMapACPITable(SDTHeader *tableHeader) {
    // First map the header so we know the size
    for (uintptr_t i = ALIGN_DOWN((uintptr_t)tableHeader, 0x1000); i < (uintptr_t)tableHeader + sizeof(SDTHeader); i += 0x1000) {
        Kernel::VMM::MemoryMap(nullptr, i, i, false);
    }
    // Then from the start of the table let's map the whole length of it
    for (uintptr_t i = ALIGN_DOWN((uintptr_t)tableHeader, 0x1000); i < (uintptr_t)tableHeader + tableHeader->Length; i += 0x1000) {
        Kernel::VMM::MemoryMap(nullptr, i, i, false);
    }
}

SDTHeader *GlobalRSDT = nullptr;
bool RSDTPassedChecksum = true;

namespace Kernel::ACPI {
    void SetRSDP(uintptr_t rsdp) {
        RSDP *SystemRSDP = (RSDP *)rsdp;
        GlobalRSDT = (SDTHeader *)(uintptr_t)SystemRSDP->RSDTPtr;

        // Map the RSDT
        Kernel::Log(KERNEL_LOG_DEBUG, "RSDP Pointer = 0x%x\n", GlobalRSDT);
        MemoryMapACPITable(GlobalRSDT);

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

            if (strncmp(Signature, (const char*)header->Signature, 4) == 0) {
                MemoryMapACPITable(header);
                return header;
            }
        }

        return nullptr;
    }
}
