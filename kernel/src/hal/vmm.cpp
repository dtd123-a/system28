/*
    * vmm.cpp
    * Implements virtual memory (paging).
    * Created 07/09/23 DanielH
*/

#include <limine.h>
#include <hal/vmm.hpp>
#include <mm/pmm.hpp>
#include <terminal/terminal.hpp>
#include <stddef.h>
#include <mm/mem.hpp>
#include <libs/kernel.hpp>
#include <early/bootloader_data.hpp>

extern BootloaderData GlobalBootloaderData;

PageTable *kernelPML4 = nullptr;

/* Tests for alignment. */
static bool IsAligned(uintptr_t addr, size_t boundary) {
    if ((addr % boundary) == 0) return true;
    return false;
}

/* Converts an HHDM virtual address to a physical address */
uintptr_t HHDMVirtToPhys(uintptr_t virt) {
    return virt - GlobalBootloaderData.hhdm_response->offset;
}

/* Converts an physical address to an HHDM virtual address */
uintptr_t HHDMPhysToVirt(uintptr_t phys) {
    return phys + GlobalBootloaderData.hhdm_response->offset;
}

static PageTable *GetNextLevel(PageTable *current_level, size_t entry) {
    if (!current_level) return nullptr;

    if (!current_level->entries[entry].Present) {
        void *new_entry = Kernel::Mem::AllocatePage();
        if (!new_entry) return nullptr;
        if (!IsAligned((uintptr_t)new_entry, 0x1000)) return nullptr;

        current_level->entries[entry].PhysicalAddr = ((uintptr_t)new_entry >> 12);
        current_level->entries[entry].Present = true;
        current_level->entries[entry].RW = true;

        return (PageTable *)HHDMPhysToVirt((uintptr_t)new_entry);
    } else return (PageTable *)(uintptr_t)HHDMPhysToVirt(current_level->entries[entry].PhysicalAddr << 12);
}

namespace Kernel::VMM {
    /* Large page is 2MiB */
    bool MemoryMap(PageTable *target_pagemap, uintptr_t virt, uintptr_t phys, bool largePage) {
        if (!target_pagemap) {
            if (!kernelPML4) {
                return false;
            }

            target_pagemap = kernelPML4;
        }

        size_t pml4_entry = (virt & ((uint64_t)0x1FF << 39)) >> 39;
        size_t pml3_entry = (virt & ((uint64_t)0x1FF << 30)) >> 30;
        size_t pml2_entry = (virt & ((uint64_t)0x1FF << 21)) >> 21;
        size_t pml1_entry = (virt & ((uint64_t)0x1FF << 12)) >> 12;
        size_t lowest_entry = pml1_entry;

        PageTable *pml3 = GetNextLevel(target_pagemap, pml4_entry);
        PageTable *pml2 = GetNextLevel(pml3, pml3_entry);
        PageTable *lowest = nullptr;

        if (!largePage) {
            lowest = GetNextLevel(pml2, pml2_entry);
        } else { 
            lowest = pml2;
            lowest_entry = pml2_entry;
            lowest->entries[lowest_entry].PageSize = true;
        }

        if (!lowest) return false;

        lowest->entries[lowest_entry].PhysicalAddr = ((uintptr_t)phys >> 12);
        lowest->entries[lowest_entry].Present = true;
        lowest->entries[lowest_entry].RW = true;

        return true;
    }
    
    void InitPaging(
        limine_memmap_response memmap,
        limine_kernel_address_response kaddr
    ) {
        void *pml4_allocation = Mem::AllocatePage();
        if (!pml4_allocation) {
            Panic("Unable to allocate memory for page map.");        
        }

        PageTable *pml4 = (PageTable *)HHDMPhysToVirt((uintptr_t)pml4_allocation);

        for (size_t i = 0; i < memmap.entry_count; i++) {
            switch (memmap.entries[i]->type) {
                case LIMINE_MEMMAP_KERNEL_AND_MODULES: {
                    for (uintptr_t j = 0; j < memmap.entries[i]->length; j += 4096) {
                        uintptr_t phys = memmap.entries[i]->base + j;
                        uintptr_t virt = phys + kaddr.virtual_base - kaddr.physical_base;

                        MemoryMap(pml4, virt, phys, false);
                    }
                    break;
                }

                default: {
                    for (uintptr_t j = 0; j < memmap.entries[i]->length; j += 4096) {
                        uintptr_t phys = memmap.entries[i]->base + j;
                        uintptr_t virt = phys + GlobalBootloaderData.hhdm_response->offset;

                        MemoryMap(pml4, virt, phys, false);
                    }

                    break;
                }
            }
        }

        kernelPML4 = pml4;
   }

    void LoadKernelCR3() {
        LoadCR3((void *)HHDMVirtToPhys((uintptr_t)kernelPML4));
    }
}
