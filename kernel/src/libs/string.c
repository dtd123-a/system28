/*
    * string.c
    * String handling code
    * Created 09/09/23
*/
#include <stddef.h>

int strncmp(const char *str1, const char *str2, int n) {
    int i = 0;

    while (i != n && *str1 && (*str1 == *str2)) {
        i++;

        str1++;
        str2++;
    }

    if (i == n) return 0;

    return *str1 - *str2;
}

int strlen(const char *str) {
    int n = 0;

    while (*str != '\0') {
        n++;
        str++;
    }

    return n;
}

const char *strcpy(char *dest, const char *src) {
    const char *tmp = dest;
    while ((*dest++ = *src++) != '\0');

    return tmp;
}
