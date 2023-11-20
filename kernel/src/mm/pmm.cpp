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
size_t PagesTracking = 0;

struct PageNode {
    PageNode *next;  
};

PageNode head = {.next = 0};

static void InsertNode(void *page) {
    PageNode *node = (PageNode *)page;

    node->next = head.next;
    head.next = node;
}

static void *RemoveNode() {
    if (head.next) {
        PageNode *ret = head.next;
        head.next = ret->next;

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
                    /* For each page */
                    for (size_t j = 0; j < mmap.entries[i]->length / 4096; j++) {
                        PagesTracking++;
                        InsertNode((void *)HHDMPhysToVirt(mmap.entries[i]->base + (j * 4096)));
                    }
                }
            }
        }

        /* Log system memory info */
        Log(KERNEL_LOG_INFO, "[PMM] Total system memory: %d MiB\n", TotalMemory / 1024 / 1024);
        Log(KERNEL_LOG_INFO, "[PMM] Usable system memory: %d MiB\n", TotalUsableMemory / 1024 / 1024);
        Log(KERNEL_LOG_INFO, "[PMM] Tracking %d physical memory pages\n", PagesTracking);
    }

    SPINLOCK_CREATE(PageAlloc_Lock);
    void *AllocatePage() {
        SpinlockAquire(&PageAlloc_Lock);

        void *page = RemoveNode();
        memset(page, 0, 0x1000);

        void *ret = (void *)HHDMVirtToPhys((uintptr_t)page);

        SpinlockRelease(&PageAlloc_Lock);
        return ret;
    }

    SPINLOCK_CREATE(PageFree_Lock);
    void FreePage(void *addr) {
        SpinlockAquire(&PageFree_Lock);

        void *virt_page = (void *)HHDMPhysToVirt((uintptr_t)addr);

        InsertNode(virt_page);

        SpinlockRelease(&PageFree_Lock);
    }
}
