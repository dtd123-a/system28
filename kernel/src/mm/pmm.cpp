/*
    * pmm.cpp
    * Physical memory manager
    * Created 02/09/2023
*/

/*
    * TODO: Rewrite all of this, there is a major flaw, which is that it only allocates from the largest block
*/

#include <limine.h>
#include <stddef.h>
#include <terminal/terminal.hpp>
#include <mm/mem.hpp>
#include <hal/vmm.hpp>
#include <libs/kernel.hpp>

namespace Kernel::Mem {
    struct Page {
        void *ptr;
        bool free : 1;
    };

    struct Page *FrameList = {0};
    uint32_t FrameListSize = 0;
    uint32_t MemSegSize = 0;

    void InitializePMM(limine_memmap_response mmap) {
        uint32_t LargestEntryLength = 0;
        limine_memmap_entry *LargestEntry = nullptr;

        for (size_t i = 0; i < mmap.entry_count; i++) {
            switch (mmap.entries[i]->type) {
                case LIMINE_MEMMAP_USABLE:
                {
                    if (mmap.entries[i]->length > LargestEntryLength) {
                        LargestEntryLength = mmap.entries[i]->length;
                        LargestEntry = mmap.entries[i];
                    }
                    break;
                }
            }
        }
        
        if (!LargestEntry) {
            Panic("No usable memory was found on the system.");
        }

        FrameList = (struct Page *)HHDMPhysToVirt(LargestEntry->base);
        uintptr_t nextAddr = ALIGN_UP((uint64_t)FrameList + (sizeof(Page) * (LargestEntry->length / 0x1000)), 0x1000);
        FrameListSize = nextAddr - (uint64_t)FrameList;

        MemSegSize = LargestEntry->length - FrameListSize;

        for (size_t i = 0; i < MemSegSize / 0x1000; i++) {
            Page page = {
                .ptr = (void *)nextAddr,
                .free = true
            };
            FrameList[i] = page;
            nextAddr += 0x1000;
        }
    }

    void *AllocatePage() {
        for (size_t i = 0; i < MemSegSize / 0x1000; i++) {
            if (FrameList[i].free) {
                FrameList[i].free = false;
                
                memset((void*)FrameList[i].ptr, 0, 0x1000);
                return FrameList[i].ptr;
            }
        }

        return nullptr;
    }

    void FreePage(void *addr) {
        for (size_t i = 0; i < MemSegSize / 0x1000; i++) {
            if ((FrameList[i].ptr = addr)) {
                FrameList[i].free = true;
            }
        }
    }
}
