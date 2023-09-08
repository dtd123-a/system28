#pragma once
#include <stdint.h>

namespace Kernel::CPU::Interrupts {    
    struct IDTEntry
    {
        uint16_t Offset0;
        uint16_t Selector;
        uint8_t Ist;
        uint8_t TypesAttrib;
        uint16_t Offset1;
        uint32_t Offset2;
        uint32_t Reserved;
    }__attribute__((packed));

    struct IDTR
    {
        uint16_t Limit;
        uint64_t Addr;
    }__attribute__((packed));
    
    struct CInterruptRegisters
    {
        uint64_t ip;
        uint64_t cs;
        uint64_t flags;
        uint64_t sp;
        uint64_t ss;
    }__attribute__((packed));

    void Initialize();
}