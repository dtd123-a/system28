#include <limine.h>
#include <stddef.h>
#include <terminal/terminal.hpp>
#include <mm/mem.hpp>

namespace Kernel::Mem {
    struct Page {
        void* ptr;
        bool free : 1;
    };

    struct Page* FrameList = {0};
    uint32_t LargestMemSegSize = 0;

    void InitializePMM(limine_memmap_response mmap) {
        uint32_t largestEntryLength = 0;
        limine_memmap_entry largestEntry;

        for (size_t i = 0; i < mmap.entry_count; i++) {
            switch (mmap.entries[i]->type) {
                case LIMINE_MEMMAP_USABLE:
                {
                    if (mmap.entries[i]->length > largestEntryLength) {
                        largestEntryLength = mmap.entries[i]->length;
                        largestEntry = *mmap.entries[i];
                    }
                }
            }
        }

        if (!largestEntry.length) {
            Kernel::Log(KERNEL_LOG_FAIL, "No usable memory found!\n");
            while(1); // TODO Replace with proper panic or other function
        }

        FrameList = (struct Page *)largestEntry.base;
        uintptr_t nextAddr = ALIGN_UP(largestEntry.base + (sizeof(Page) * (largestEntry.length / 0x1000)), 0x1000);

        for (size_t i = 0; i < largestEntry.length / 0x1000; i++) {
            Page page = {
                .ptr = (void *)nextAddr,
                .free = true
            };
            FrameList[i] = page;
            nextAddr += 0x1000;
        }

        LargestMemSegSize = largestEntry.length;
    }

    void* AllocatePage() {
        for (size_t i = 0; i < LargestMemSegSize / 0x1000; i++) {
            if (FrameList[i].free) {
                FrameList[i].free = false;

                ::memset((char*)FrameList[i].ptr, 0, 0x1000);
                return FrameList[i].ptr;
            }
        }

        return nullptr;
    }

    void FreePage(void *addr) {
        for (size_t i = 0; i < LargestMemSegSize / 0x1000; i++) {
            if ((FrameList[i].ptr = addr)) {
                FrameList[i].free = true;
            }
        }
    }
};
