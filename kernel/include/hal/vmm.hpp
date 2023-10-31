/*
    * vmm.hpp
    * Implements virtual memory (paging).
    * Created 07/09/23 DanielH
*/
#pragma once
#include <stdint.h>
#include <limine.h>

struct PageTableEntry {
    uintptr_t Present : 1;
    uintptr_t RW : 1;
    uintptr_t User : 1;
    uintptr_t WriteThrough : 1;
    uintptr_t CacheDisable : 1;
    uintptr_t Accessed : 1;
    uintptr_t Ignored : 1;
    uintptr_t PageSize : 1;
    uintptr_t Ignored1 : 4;
    uintptr_t PhysicalAddr : 40;
    uintptr_t Reserved : 12;
}__attribute__((packed));

struct PageTable {
    PageTableEntry entries[512];
}__attribute__((packed)) __attribute__((aligned(0x1000)));

extern "C" void LoadCR3(void *pml4);

namespace Kernel::VMM {
    void InitPaging(limine_memmap_response memmap, limine_kernel_address_response kaddr, uintptr_t hhdm_base);
    void LoadKernelCR3();
    bool MemoryMap(PageTable *target_pagemap, uintptr_t virt, uintptr_t phys, bool largePage);
}