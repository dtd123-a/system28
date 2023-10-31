/*
    * pmm.hpp
    * Physical memory manager
    * Created 02/09/2023
*/

#pragma once
#include <limine.h>

namespace Kernel::Mem {
    void InitializePMM(limine_memmap_response mmap);
    void* AllocatePage();
    void FreePage(void *addr);
};
