/*
    * acpi.hpp
    * ACPI table handling code
    * Created 09/09/2023
*/
#pragma once
#include <stdint.h>
#include <stddef.h>

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
        enum {
            GAS_TYPE_MMIO = 0x00,
            GAS_TYPE_IO = 0x01,
            GAS_TYPE_PCI = 0x02,
            GAS_TYPE_EMBEDDED_CONTROLLER = 0x03,
            GAS_TYPE_SMBUS = 0x04,
            GAS_TYPE_SYS_CMOS = 0x05,
            GAS_TYPE_PCI_BAR_TARGET = 0x06,
            GAS_TYPE_IPMI = 0x07,
            GAS_TYPE_GPIO = 0x08,
            GAS_TYPE_GENERIC_SERIAL_BUS = 0x09,
            GAS_TYPE_PLATFORM_COMMUNICATIONS_CHANNEL = 0x0A,
            /* 0x0B to 0x7E reserved */
            GAS_TYPE_FUNCTIONAL_FIXED_HW = 0x7F,
            /* 0x80 to 0xBF reserved */
            /* 0xC0 to 0xFF OEM defined */
        };

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
        uint32_t FirmwareCtrl;
        uint32_t DSDT;

        // ACPI 1.0 only
        uint8_t Reserved;

        uint8_t PreferredPowerManagementProfile;
        uint16_t SCIInterrupt;
        uint32_t SCICmd;
        uint8_t ACPIEnable;
        uint8_t ACPIDisable;
        uint8_t S4BIOS_REQ;
        uint8_t PSTATE_CNT;
        uint32_t PM1a_EVT_BLK;
        uint32_t PM1b_EVT_BLK;
        uint32_t PM1a_CNT_BLK;
        uint32_t PM1b_CNT_BLK;
        uint32_t PM2_CNT_BLK;
        uint32_t PM_TMR_BLK;
        uint32_t GPE0_BLK;
        uint32_t GPE1_BLK;
        uint8_t PM1_EVT_LEN;
        uint8_t PM1_CNT_LEN;
        uint8_t PM2_CNT_LEN;
        uint8_t PM_TMR_LEN;
        uint8_t GPE0_BLK_LEN;
        uint8_t GPE1_BLK_LEN;
        uint8_t GPE1_BASE;
        uint8_t CST_CNT;
        uint16_t P_LVL2_LAT;
        uint16_t P_LVL3_LAT;
        uint16_t FlushSize;
        uint16_t FlushStride;
        uint8_t DutyOffset;
        uint8_t DutyWidth;
        uint8_t DayAlarm;
        uint8_t MonthAlarm;
        uint8_t Century;
        uint16_t IAPC_BOOT_ARCH;
        uint8_t Reserved_0;
        uint32_t Flags;
        // Byte off. 116
        GenericAddressStructure ResetReg;
        // Byte off. 128
        uint8_t ResetValue; // The value to place in ResetReg for a reset
        uint16_t ARM_BOOT_ARCH;
        uint8_t FADTMinorVersion;
        uint64_t X_FirmwareCtrl;
        uint64_t X_DSDT;
        GenericAddressStructure X_PM1a_EVT_BLK;
        GenericAddressStructure X_PM1b_EVT_BLK;
        GenericAddressStructure X_PM1a_CNT_BLK;
        GenericAddressStructure X_PM1b_CNT_BLK;
        GenericAddressStructure X_PM2_CNT_BLK;
        GenericAddressStructure X_PM_TMR_BLK;
        GenericAddressStructure X_GPE0_BLK;
        GenericAddressStructure X_GPE1_BLK;
        GenericAddressStructure SleepCtrlRegister;
        GenericAddressStructure SleepStatRegister;
        GenericAddressStructure HypervisorId;
    }__attribute__((packed));

    SDTHeader *GetACPITable(const char *Signature);
    void InitializeACPI(uintptr_t rsdp);
    bool PMTMRSleep(size_t us);
    bool PerformACPIReboot();
}