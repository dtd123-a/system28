/*
    * mem.c
    * Memory functions
    * Created 02/09/2023
*/
#include <stddef.h>

void *memcpy(char *destination, char *source, size_t n)
{
    size_t i = 0;

    while (i < n) {
        destination[i] = source[i];

        i++;
    }

    return destination;
}

void *memset(void *destination, int val, size_t n)
{
    volatile unsigned char *buf = (volatile unsigned char *)destination;

    for (size_t i = 0; i < n; i++) {
        buf[i] = (unsigned char)val;
    }

    return destination;
}
