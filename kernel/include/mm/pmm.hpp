#pragma once
#include <limine.h>

namespace Kernel::Mem {
    void InitializePMM(limine_memmap_response mmap);
    void* AllocatePage();
    void FreePage(void *addr);
};
