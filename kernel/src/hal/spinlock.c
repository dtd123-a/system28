/*
    * spinlock.c
    * Spinlock implementation
    * Created 02/09/23 DanielH
*/
#include <stdatomic.h>

extern void CPUNop();

void SpinlockAquire(volatile int* lock)
{
    while (atomic_flag_test_and_set_explicit(lock, memory_order_acquire)) {
        CPUNop();   // TODO Once timer interrupts are up and running for all the CPUs then use
                    // CPUHlt() instead, as right now it won't work as halt pauses all execution
                    // until next interrupt.
    }
}

void SpinlockRelease(volatile int* lock)
{
    atomic_flag_clear_explicit(lock, memory_order_release);
}