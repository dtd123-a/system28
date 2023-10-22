/*
    * kernel.hpp
    * Implements some kernel panic functions.
    * Created 12/09/23 DanielH
*/
#pragma once
#include <hal/cpu/interrupt/idt.hpp>
#include <mm/heap.hpp>

namespace Kernel {
    __attribute__((noreturn)) void Panic(const char *error);
    __attribute__((noreturn)) void PanicFromException(CPU::Interrupts::CInterruptRegisters *registers, int error_code);

    namespace Lib {
        // A custom (not implementing std::vector, but still modelled after it) vector class.
        template <typename T> class Vector {
            // Private data
            T *Array = nullptr;
            size_t CurrentCapacity = 0;
            size_t CurrentElementCount = 0;

public:
            Vector() {
                Array = new T;
                CurrentCapacity = 1;
            }

            ~Vector() {
                delete Array;
            }

            // Adds an element to the end
            void push_back(T value) {
                if (CurrentCapacity == CurrentElementCount) { // The vector is currently full
                    Array = (T *)Mem::Reallocate((void *)Array, sizeof(T) * (CurrentCapacity * 2)); // Allocate double the size.
                    CurrentCapacity *= 2;
                }

                Array[CurrentElementCount] = value;
                CurrentElementCount++;
            }

            T & at(size_t index) {
                if (index < CurrentCapacity) {
                    return Array[index];
                } else {
                    Panic("[KERNEL BUG] Vector out of bounds in kernel mode!");
                }
            }

            void pop_back() {
                CurrentElementCount--;
            }

            uint16_t size() {
                return CurrentElementCount;
            }

            void front() {
                return at(0);
            }

            void back() {
                return at(CurrentElementCount - 1);
            }

            T *data() {
                return Array;
            }

            T & operator[](size_t index) {
                return at(index);
            }

            void remove(size_t index) {
                if (index >= CurrentElementCount) {
                    Panic("[KERNEL BUG] Vector out of bounds in kernel mode!");
                }

                for (size_t i = index; i < CurrentElementCount; i++) {
                    at(i) = at(i + 1);
                }

                CurrentElementCount--;
            }
        };
    }

    namespace IO {
        static inline void outb(uint16_t port, uint8_t val) {
            asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port) );
        }

        static inline void outw(uint16_t port, uint16_t val) {
            asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port) );
        }

        static inline void outl(uint16_t port, uint32_t val) {
            asm volatile ("outl %0, %1" : : "a"(val), "Nd"(port) );
        }

        static inline uint8_t inb(uint16_t port) {
            uint8_t ret;
            asm volatile ("inb %1, %0"
                        : "=a"(ret)
                        : "Nd"(port) );
            return ret;
        }

        static inline uint16_t inw(uint16_t port) {
            uint16_t ret;
            asm volatile ("inw %1, %0"
                        : "=a"(ret)
                        : "Nd"(port) );
            return ret;
        }

        static inline uint32_t inl(uint16_t port) {
            uint32_t ret;
            asm volatile ("inl %1, %0"
                        : "=a"(ret)
                        : "Nd"(port) );
            return ret;
        }

        static inline void io_timeout() {
            outb(0x80, 0);
        }
    }
}
