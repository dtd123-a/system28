/*
    * heap.cpp
    * Implements a heap memory allocator.
    * Created 16/09/23 DanielH
*/

#include <stddef.h>
#include <mm/pmm.hpp>
#include <mm/mem.hpp>
#include <mm/heap.hpp>
#include <terminal/terminal.hpp>
#include <hal/spinlock.hpp>
#include <hal/vmm.hpp>

struct Node {
    size_t size;

    Node *next;
};

static Node head = {0, 0};

static void InsertNode(void *base, size_t size) {
    // Insert at the very beginning
    Node *node = (Node *)base;

    node->size = size;
    node->next = head.next;
    head.next = node;
}

static size_t ExpandHeap(size_t pageCount) {
    /* The physical memory manager returns physical memory addresss. */
    void *start = (void *)HHDMPhysToVirt((uintptr_t)Kernel::Mem::AllocatePage());
    if (!start) return 0;

    size_t i;
    for (i = 0; i < pageCount - 1; i++) {
        if (!Kernel::Mem::AllocatePage()) break;
    }

    InsertNode(start, (i + 1) * 4096);
    return i + 1;
}

static Node *FindSuitableNode(size_t size) {
    Node *current = &head;

    while (current->next != nullptr) {
        if (current->next->size > size) {
            if (current->next->size >= sizeof(Node) + size) {
                Node *ret = current->next;
                current->next = current->next->next;

                size_t requiredSize = sizeof(Node) + size;
                uintptr_t nodeTop = (uintptr_t)ret + sizeof(Node) + size;
                uintptr_t extraSize = ret->size - requiredSize;

                if (extraSize) InsertNode((void *)nodeTop, extraSize);

                ret->size = requiredSize;

                return ret;
            }

        }
        current = current->next;
    }

    // No suitable node found
    if (!ExpandHeap(10)) return nullptr; // Expand by 10 pages (0x1000 * 10), if that memory is not available we will get more anyway.
    return FindSuitableNode(size);
}

namespace Kernel::Mem {
    void InitializeHeap(size_t heapSize) {
        if (!heapSize) return;
        size_t pages = heapSize / 4096;

        if (ExpandHeap(pages) < pages) {
            Log(KERNEL_LOG_DEBUG, "Warning: Initial heap size request was not met (%d pages).", heapSize);
        }
    }

    SPINLOCK_CREATE(malloc_spinlock);
    void *Allocate(size_t size) {
        SpinlockAquire(&malloc_spinlock);
        Node *node = FindSuitableNode(size);
        SpinlockRelease(&malloc_spinlock);

        return (void *)((uintptr_t)node + sizeof(Node));
    }

    SPINLOCK_CREATE(free_spinlock);
    void Free(void *base) {
        SpinlockAquire(&free_spinlock);
        Node *node = (Node *)((uintptr_t)base - sizeof(Node));
        InsertNode((void *)node, node->size);
        SpinlockRelease(&free_spinlock);
    }

    SPINLOCK_CREATE(realloc_spinlock);
    void *Reallocate(void *object, size_t new_size) {
        SpinlockAquire(&realloc_spinlock);

        /* Gets the object's frame struct in the freelist (it is placed right before the block actually starts)*/
        Node *node = (Node *)((uintptr_t)object - sizeof(Node));
        /* Size of block is calculated by requested size + sizeof(Node) to fit both the frame and the block */
        size_t old_size = node->size - sizeof(Node); 

        void *new_object = Allocate(new_size);
        if (!new_object) {
            goto cleanup;
        }

        memcpy(new_object, object, old_size);

        Free(object);

cleanup:
        SpinlockRelease(&realloc_spinlock);
        return new_object;
    }
}
