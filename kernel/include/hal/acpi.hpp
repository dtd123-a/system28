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

    struct GenericAddressStructure {
        /*
            * The Generic Address Structure (GAS) is used by ACPI to indicate where the I/O or data is.
        */
        uint8_t AddressSpace;
        uint8_t BitWidth;
        uint8_t BitOffset;
        uint8_t AccessSize;
        uint64_t Address;
    }__attribute__((packed));

    struct FADTStructure : SDTHeader {
        /*
            * Thanks a lot to https://wiki.osdev.org/FADT for this extremely long C struct definition.
        */

        uint32_t FirmwareCtrl;
        uint32_t DSDT;

        // ACPI 1.0 only
        uint8_t Reserved;

        uint8_t PreferredPowerManagementProfile;
        uint16_t SCIInterrupt;
        uint32_t SMICommandPort;
        uint8_t AcpiEnable;
        uint8_t AcpiDisable;
        uint8_t S4BIOS_REQ;
        uint8_t PSTATE_Control;
        uint32_t PM1aEventBlock;
        uint32_t PM1bEventBlock;
        uint32_t PM1aControlBlock;
        uint32_t PM1bControlBlock;
        uint32_t PM2ControlBlock;
        uint32_t PMTimerBlock;
        uint32_t GPE0Block;
        uint32_t GPE1Block;
        uint8_t PM1EventLength;
        uint8_t PM1ControlLength;
        uint8_t PM2ControlLength;
        uint8_t PMTimerLength;
        uint8_t GPE0Length;
        uint8_t GPE1Length;
        uint8_t GPE1Base;
        uint8_t CStateControl;
        uint16_t WorstC2Latency;
        uint16_t WorstC3Latency;
        uint16_t FlushSize;
        uint16_t FlushStride;
        uint8_t DutyOffset;
        uint8_t DutyWidth;
        uint8_t DayAlarm;
        uint8_t MonthAlarm;
        uint8_t Century;
    
        // ACPI 2+ only
        uint16_t BootArchitectureFlags;
    
        uint8_t Reserved2;
        uint32_t Flags;
    
        GenericAddressStructure ResetReg;
    
        uint8_t ResetValue;
        uint8_t Reserved3[3];
    
        // ACPI 2+ only
        uint64_t X_FirmwareControl;
        uint64_t X_Dsdt;
    
        GenericAddressStructure X_PM1aEventBlock;
        GenericAddressStructure X_PM1bEventBlock;
        GenericAddressStructure X_PM1aControlBlock;
        GenericAddressStructure X_PM1bControlBlock;
        GenericAddressStructure X_PM2ControlBlock;
        GenericAddressStructure X_PMTimerBlock;
        GenericAddressStructure X_GPE0Block;
        GenericAddressStructure X_GPE1Block;
    }__attribute__((packed));

    void SetRSDP(uintptr_t rsdp);
    SDTHeader *GetACPITable(const char *Signature);
    void InitializeACPI();
}