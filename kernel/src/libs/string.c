/*
    * string.cpp
    * String handling code
    * Created 09/09/23
*/
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
