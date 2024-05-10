/*
    * terminal.cpp
    * Implements terminal functions
    * Created 01/09/2023 DanielH
*/

#include <stdint.h>
#include <stdarg.h>
#include <terminal/terminal.hpp>
#include <early/bootloader_data.hpp>

/* Flanterm library header files */
#include "../external_libs/flanterm/backends/fb.h"
#include "../external_libs/flanterm/flanterm.h"

#include <hal/spinlock.hpp>
#include <hal/cpu/interrupt/idt.hpp>
#include <hal/debug/serial.hpp>

flanterm_context* fb_ctx = nullptr;

Kernel::Debug::SerialPort *TargetPort = nullptr;

extern BootloaderData GlobalBootloaderData;

static char* itoa(int num, char* str, int base, int n) {
    if (n == 0) {
        return NULL;
    }

    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }

    int is_negative = 0;
    if (num < 0 && base == 10) {
        is_negative = 1;
        num = -num;
    }

    int i = 0;
    while (num != 0 && i < n - 1) {
        int digit = num % base;
        str[i++] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        num /= base;
    }

    if (is_negative && i < n - 1) {
        str[i++] = '-';
    }

    str[i] = '\0';

    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }

    return str;
}

static char* hexToString(uint64_t value, char* buffer, size_t n) {
    const char* hexChars = "0123456789ABCDEF";
    char* ptr = &buffer[n - 1];
    *ptr = '\0';

    if (value == 0)
    {
        if (n > 1)
            *--ptr = '0';
    }
    else
    {
        while (value > 0 && ptr > buffer)
        {
            *--ptr = hexChars[value & 0xF];
            value >>= 4;
        }
    }

    return ptr;
}


namespace Kernel {
    SPINLOCK_CREATE(PutCharLock);
    void PutChar(char c) {
        SpinlockAquire(&PutCharLock);
        flanterm_write(fb_ctx, &c, 1);
        if (TargetPort) {
            if (c == '\n') {
                TargetPort->WriteCharacter('\r');
            }
            TargetPort->WriteCharacter(c);
        }
        SpinlockRelease(&PutCharLock);
    }
    
    namespace Init {
        void InitializeFlanterm(uint32_t *framebuffer, int width, int height, int pitch, int red_mask_size, int red_mask_shift, int green_mask_size, int green_mask_shift, int blue_mask_size, int blue_mask_shift) {
            /* The flanterm_fb_simple_init has been removed upstream */
            /* Instead, we use the advanced init function */
            fb_ctx = flanterm_fb_init(
                NULL,
                NULL,
                framebuffer, width, height, pitch,
                red_mask_size, red_mask_shift,
                green_mask_size, green_mask_shift,
                blue_mask_size, blue_mask_shift,
                NULL,
                NULL, NULL,
                NULL, NULL,
                NULL, NULL,
                NULL, 0, 0, 1,
                0, 0,
                0
            );
        }

        void SetSerialOutputPort(Debug::SerialPort *port) {
            TargetPort = port;
        }
    }

    SPINLOCK_CREATE(PrintLock);
    void Print(const char *string)
    {
        SpinlockAquire(&PrintLock);
        while (*string != '\0') {
            PutChar(*string);
            string++;
        }
        SpinlockRelease(&PrintLock);
    }

    SPINLOCK_CREATE(LogSpinlock);
    void Log(KernelLogType type, const char *format, ...)
    {
        SpinlockAquire(&LogSpinlock);
        
        switch (type) {
            case KERNEL_LOG_SUCCESS:
                Print("LOG  \x1B[32mOK    \x1B[0m ");
                break;
            case KERNEL_LOG_FAIL:
                Print("LOG  \x1B[31mFAIL  \x1B[0m ");
                break;
            case KERNEL_LOG_INFO:
                Print("LOG  \x1B[94mINFO  \x1B[0m ");
                break;
            case KERNEL_LOG_DEBUG:
                Print("LOG  \x1b[38;5;48mDEBUG \x1B[0m ");
                break;
            case KERNEL_LOG_EVENT:
                Print("LOG  \x1b[0;36mEVENT \x1B[0m ");
                break;
            default:
                break;
        }

        va_list args;
        va_start(args, format);

        while (*format != '\0') {
            if (*format == '%') {
                format++;
                
                if (*format == 'c') {
                    int c = va_arg(args, int);
                    PutChar((char)c);
                }
                else if (*format == 'd') {
                    int n = va_arg(args, int);
                    char buff[128];
                    itoa(n, buff, 10, 128);
                    Print(buff);
                }
                else if (*format == 'x') {
                    uintptr_t n = va_arg(args, uintptr_t);
                    char buff[128];
                    Print(hexToString(n, buff, 128));
                }
                else if (*format == 's') {
                    const char *str = va_arg(args, const char*);
                    Print(str);
                }

                format++;
            }
            else {
                PutChar(*format);
                format++;
            }
        }

        va_end(args);
        SpinlockRelease(&LogSpinlock);
    }
}
