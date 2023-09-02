/*
    * terminal.cpp
    * Implements terminal functions
    * Created 01/09/23 DanielH
*/

#include <stdint.h>
#include <stdarg.h>
#include <terminal/terminal.hpp>
#include <external_libs/flanterm/flanterm.h>
#include <external_libs/flanterm/backends/fb.h>

flanterm_context* fb_ctx = nullptr;

void PutChar(char c)
{
    flanterm_write(fb_ctx, &c, 1);
}

static char* itoa(int num, char* str, int base, int n)
{
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

static char* hexToString(uint64_t value, char* buffer, size_t n)
{
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
    namespace Init {
        void InitializeFlanterm(uint32_t *framebuffer, int width, int height, int pitch)
        {
            fb_ctx = flanterm_fb_simple_init(framebuffer, width, height, pitch);
        }
    }

    void Print(const char *string)
    {
        while (*string != '\0') {
            PutChar(*string);
            string++;
        }
    }

    void Log(KernelLogType type, const char *format, ...)
    {
        switch (type) {
            case KERNEL_LOG_SUCCESS:
                Print("[\x1B[32m OK \x1B[0m] ");
                break;
            case KERNEL_LOG_FAIL:
                Print("[\x1B[31m FAIL \x1B[0m] ");
                break;
            case KERNEL_LOG_INFO:
                Print("[\x1B[94m INFO \x1B[0m] ");
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

                format++;
            }
            else {
                PutChar(*format);
                format++;
            }
        }

        va_end(args);
    }
}
