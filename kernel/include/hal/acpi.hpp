/*
    * acpi.hpp
    * ACPI table handling code
    * Created 09/09/23
*/
#pragma once
#include <stdint.h>

namespace Kernel::ACPI {
    struct SDTHeader {
        char Signature[4];
        uint32_t Length;
        uint8_t Revision;
        uint8_t Checksum;
        char OEMId[6];
        char OEMTableId[8];
        uint32_t OEMRevision;
        uint32_t CreatorId;
        uint32_t CreatorRevision;
    }__attribute__((packed));

    void SetRSDP(uintptr_t rsdp);
    SDTHeader *GetACPITable(const char *Signature);
}