/*
    * gdt.cpp
    * Implements Global Descriptor Table (GDT) for x86_64
    * Created 02/09/2023 DanielH
*/

#include <hal/cpu/gdt.hpp>

using namespace Kernel::CPU::GDT;

GDTStructure GDT = {
    .Null = {0, 0, 0, 0, 0, 0},
    .KernelCode = {0xFF, 0, 0, 0x9A, 0xA0, 0},
    .KernelData = {0xFF, 0, 0, 0x92, 0xC0, 0},
    .UserCode = {0xFF, 0, 0, 0xFA, 0xA0, 0},
    .UserData = {0xFF, 0, 0, 0xF2, 0xC0, 0}
};

GDTR GDTPtr = {
    .Size = sizeof(GDT) - 1,
    .Addr = (uintptr_t)&GDT
};

void InstallGDT() {
    LoadGDT(&GDTPtr); 
}

namespace Kernel::CPU::GDT {
    void Load() {
        InstallGDT();
    }
}