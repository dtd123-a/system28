/*
    * spinlock.c
    * Spinlock implementation
    * Created 02/09/2023 DanielH
*/

/*
    May 10th, 2024 - Refined for Clang compiler support
*/

#include <stdatomic.h>

extern void CPUPause();

struct Lock {
    _Atomic volatile uint32_t _Value;
};

void SpinlockAquire(struct Lock* lock)
{
    while (atomic_flag_test_and_set_explicit(lock, memory_order_acquire)) {
        CPUPause();
    }
}

void SpinlockRelease(struct Lock *lock)
{
    atomic_flag_clear_explicit(lock, memory_order_release);
}
