extern "C" void *memcpy(char *destination, char *source, int n)
{
    int i = 0;

    while (i < n) {
        destination[i] = source[i];

        i++;
    }

    return destination;
}

extern "C" void *memset(char* destination, int val, int n)
{
    int i = 0;

    while (i < n) {
        destination[i] = val;

        i++;
    }

    return destination;
}
