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

static int ExpandHeap(size_t pageCount) {
    void *start = Kernel::Mem::AllocatePage();
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

                InsertNode((void *)nodeTop, extraSize);

                ret->size = requiredSize;

                return ret;
            }
        }
    }

    // No suitable node found
    if (!ExpandHeap(10)) return nullptr; // Expand by 10 pages (0x1000 * 10), if that memory is not available we will get more anyway.
    return FindSuitableNode(size);
}

namespace Kernel::Mem {
    void InitializeHeap(size_t heapSize) {
        if (!heapSize) return;
        head.next = (Node *)AllocatePage();

        size_t i;
        for (i = 0; i < heapSize - 4096; i += 4096) {
            if (!AllocatePage()) {
                Kernel::Log(KERNEL_LOG_FAIL, "Warning, initial heap size request was not met.");
                break;
            }
        }
        head.next->size = i + 4096;
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
}
