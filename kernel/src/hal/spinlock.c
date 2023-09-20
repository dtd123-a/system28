/*
    * spinlock.c
    * Spinlock implementation
    * Created 02/09/23 DanielH
*/
#include <stdatomic.h>

extern void CPUPause();

void SpinlockAquire(volatile int* lock)
{
    while (atomic_flag_test_and_set_explicit(lock, memory_order_acquire)) {
        CPUPause();
    }
}

void SpinlockRelease(volatile int *lock)
{
    atomic_flag_clear_explicit(lock, memory_order_release);
}
