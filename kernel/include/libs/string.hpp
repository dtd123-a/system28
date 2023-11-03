/*
    * string.hpp
    * String handling code
    * Created 09/09/23
*/
#pragma once

extern "C" int strncmp(const char *str1, const char *str2, int n);
extern "C" int strlen(const char *str);
extern "C" int strnlen(const char *str, int n);
extern "C" const char *strcpy(char *dest, const char *src);
