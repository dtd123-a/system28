/*
    * spinlock.h
    * Spinlock implementation
    * Created 02/09/2023 DanielH
*/
#pragma once
#include <stdint.h>

#define SPINLOCK_CREATE(name) volatile uint32_t name = false

extern "C" void SpinlockAquire(volatile uint32_t* lock);
extern "C" void SpinlockRelease(volatile uint32_t* lock);
