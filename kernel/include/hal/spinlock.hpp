/*
    * spinlock.h
    * Spinlock implementation
    * Created 02/09/23 DanielH
*/
#pragma once

#define SPINLOCK_CREATE(name) volatile bool name = false

extern "C" void SpinlockAquire(volatile bool* lock);
extern "C" void SpinlockRelease(volatile bool* lock);
