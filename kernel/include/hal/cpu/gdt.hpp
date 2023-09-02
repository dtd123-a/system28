/*
    * gdt.cpp
    * Declarations for Global Descriptor Table (GDT)
    * Created 02/09/23 DanielH
*/

namespace Kernel::CPU::GDT {
    #include <stdint.h>

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

    extern "C" void LoadGDT(GDTR *);
    void Initialize();
}