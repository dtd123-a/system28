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
            switch (mmap.entries[i]->type) {
                case LIMINE_MEMMAP_USABLE: {
                    /* For each page */
                    for (size_t j = 0; j < mmap.entries[i]->length / 4096; j++) {
                        InsertNode((void *)HHDMPhysToVirt(mmap.entries[i]->base + (j * 4096)));
                    }
                }
            }
        }
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
