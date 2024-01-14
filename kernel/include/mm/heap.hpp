/*
    * heap.hpp
    * Implements a heap memory allocator.
    * Created 16/09/2023 DanielH
*/
#include <stddef.h>

namespace Kernel::Mem {
    void InitializeHeap(size_t heapSize);
    void *Allocate(size_t size);
    void Free(void *base);
    void *Reallocate(void *object, size_t new_size);
}