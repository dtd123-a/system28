/*
    * gdt.cpp
    * Declarations for Global Descriptor Table (GDT)
    * Created 02/09/2023 DanielH
*/

#include <stdint.h>

namespace Kernel::CPU::GDT {
    struct SegmentDescriptor
    {
        uint16_t Limit;
        uint16_t Base0;
        uint8_t Base1;
        uint8_t Access;
        uint8_t Granularity;
        uint8_t Base2;
    }__attribute__((packed));

    struct GDTStructure
    {
        SegmentDescriptor Null;
        SegmentDescriptor KernelCode;
        SegmentDescriptor KernelData;
        SegmentDescriptor UserCode;
        SegmentDescriptor UserData;
    }__attribute__((packed));

    struct GDTR
    {
        uint16_t Size;
        uint64_t Addr;
    }__attribute__((packed));

    /*
        NOTE: Expects CS selector to be 0x08 and DS selector to be 0x10
    */
    extern "C" void LoadGDT(GDTR *);
    void Load();
}