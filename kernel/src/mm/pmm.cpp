/*
    * pmm.cpp
    * Physical memory manager
    * Created 02/09/2023
    * Rewritten 19/11/2023
*/

#include <limine.h>
#include <stddef.h>
#include <mm/mem.hpp>
#include <hal/vmm.hpp>
#include <hal/spinlock.hpp>
#include <terminal/terminal.hpp>

/* System memory information */
size_t TotalMemory = 0;
size_t TotalUsableMemory = 0;

struct PageNode {
    size_t size;
    PageNode *next;
};

PageNode head = {.size = 0, .next = 0};

static void InsertNode(void *block, size_t size) {
    PageNode *node = (PageNode *)block;

    node->size = size;
    node->next = head.next;
    head.next = node;
}

static void *RemovePage() {
    if (head.next) {
        PageNode *ret = head.next;
        head.next = ret->next;

        if (ret->size > 4096) {
            InsertNode((void *)((uintptr_t)ret + 4096), ret->size - 4096);
            ret->size = 4096;
        }

        return ret;
    }

    return nullptr;
}

namespace Kernel::Mem {
    void InitializePMM(limine_memmap_response mmap) {
        for (size_t i = 0; i < mmap.entry_count; i++) {
            TotalMemory += mmap.entries[i]->length;
            switch (mmap.entries[i]->type) {
                case LIMINE_MEMMAP_USABLE: {
                    TotalUsableMemory += mmap.entries[i]->length;
                    if (mmap.entries[i]->length >= 4096) {
                        InsertNode((void *)HHDMPhysToVirt(mmap.entries[i]->base), mmap.entries[i]->length);
                    }
                }
            }
        }

        /* Log system memory info */
        Log(KERNEL_LOG_INFO, "[PMM] Total system memory: %d MiB\n", TotalMemory / 1024 / 1024);
        Log(KERNEL_LOG_INFO, "[PMM] Usable system memory: %d MiB\n", TotalUsableMemory / 1024 / 1024);
    }

    SPINLOCK_CREATE(PageAlloc_Lock);
    void *AllocatePage() {
        SpinlockAquire(&PageAlloc_Lock);

        void *page = RemovePage();
        memset(page, 0, 0x1000);

        void *ret = (void *)HHDMVirtToPhys((uintptr_t)page);

        SpinlockRelease(&PageAlloc_Lock);
        return ret;
    }

    SPINLOCK_CREATE(PageFree_Lock);
    void FreePage(void *addr) {
        SpinlockAquire(&PageFree_Lock);

        void *virt_page = (void *)HHDMPhysToVirt((uintptr_t)addr);

        InsertNode(virt_page, 0x1000);

        SpinlockRelease(&PageFree_Lock);
    }
}
