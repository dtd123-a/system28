/*
    * gdt.cpp
    * Implements Global Descriptor Table (GDT) for x86_64
    * Created 02/09/23 DanielH
*/

#include <hal/cpu/gdt.hpp>

using namespace Kernel::CPU::GDT;

class GlobalDescriptorTable {
    public:

    GDTStructure GDT;
    GDTR GDTPtr;

    // Note: We only set access and granularity as base and limit are obsolete in long mode
    void Setup()
    {
        // Setup default GDT values
        GDT.Null = {0, 0, 0, 0, 0, 0};
        GDT.KernelCode = {0xFF, 0, 0, 0x9A, 0xA0, 0};
        GDT.KernelData = {0xFF, 0, 0, 0x92, 0xC0, 0};
        GDT.UserCode = {0xFF, 0, 0, 0xFA, 0xA0, 0};
        GDT.UserData = {0xFF, 0, 0, 0xF2, 0xC0, 0};

        GDTPtr.Size = sizeof(GDT) - 1;
        GDTPtr.Addr = (uint64_t)&GDT;
    }
};

GlobalDescriptorTable GlobalGDT;

namespace Kernel::CPU::GDT {
    void Initialize()
    {
        GlobalGDT.Setup();
        LoadGDT(&GlobalGDT.GDTPtr);
    }
}