#include <limine.h>
#include <hal/vmm.hpp>
#include <mm/pmm.hpp>
#include <terminal/terminal.hpp>
#include <stddef.h>
#include <mm/mem.hpp>

extern "C" void LoadCR3(void *pml4);
PageTable *kernelPML4 = nullptr;
bool pagingInitialized = false;

static bool is_aligned(uintptr_t addr, size_t boundary)
{
    if ((addr % boundary) == 0) return true;
    return false;
}

static PageTable *GetNextLevel(PageTable *current_level, size_t entry) {
    if (!current_level) return nullptr;

    if (!current_level->entries[entry].Present) {
        void *new_entry = Kernel::Mem::AllocatePage();
        if (!new_entry) return nullptr;
        if (!is_aligned((uintptr_t)new_entry, 0x1000)) return nullptr;

        current_level->entries[entry].PhysicalAddr = ((uintptr_t)new_entry >> 12);
        current_level->entries[entry].Present = true;
        current_level->entries[entry].RW = true;

        return (PageTable *)new_entry;
    } else return (PageTable *)(uintptr_t)(current_level->entries[entry].PhysicalAddr << 12);
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
        limine_kernel_address_response kaddr,
        uintptr_t hhdm_base
    ) {
        PageTable *pml4 = (PageTable *)Kernel::Mem::AllocatePage();

        for (size_t i = 0; i < memmap.entry_count; i++) {
            switch (memmap.entries[i]->type) {
                case LIMINE_MEMMAP_KERNEL_AND_MODULES: {
                    Kernel::Log(KERNEL_LOG_INFO, "Mapping kernel starting from 0x%x to 0x%x!\n", memmap.entries[i]->base, memmap.entries[i]->base + kaddr.virtual_base - kaddr.physical_base);
                    for (uintptr_t j = 0; j < memmap.entries[i]->length; j += 4096) {
                        uintptr_t phys = memmap.entries[i]->base + j;
                        uintptr_t virt = phys + kaddr.virtual_base - kaddr.physical_base;

                        MemoryMap(pml4, virt, phys, false);
                    }
                    break;
                }

                case LIMINE_MEMMAP_USABLE: {
                    // For the usable memory let's identity map it.
                    Kernel::Log(KERNEL_LOG_INFO, "Mapping usable memory starting from 0x%x to 0x%x!\n", memmap.entries[i]->base, memmap.entries[i]->base);
                    for (uintptr_t j = 0; j < memmap.entries[i]->length; j += 4096) {
                        MemoryMap(pml4, memmap.entries[i]->base + j, memmap.entries[i]->base + j, false);
                    }

                    break;
                }

                default: {
                    Kernel::Log(KERNEL_LOG_INFO, "Mapping memory starting from 0x%x to 0x%x!\n", memmap.entries[i]->base, memmap.entries[i]->base + hhdm_base);
                    for (uintptr_t j = 0; j < memmap.entries[i]->length; j += 4096) {
                        uintptr_t phys = memmap.entries[i]->base + j;
                        uintptr_t virt = phys + hhdm_base;

                        MemoryMap(pml4, virt, phys, false);
                    }

                    break;
                }
            }
        }

        kernelPML4 = pml4;
        LoadCR3(pml4);
        pagingInitialized = true;
    }
}
