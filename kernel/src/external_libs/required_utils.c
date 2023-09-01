/*
    * required_utils.c
    * Implements things required by external libraries
    * Created 01/09/23
*/

void* memcpy(char* destination, char* source, int n)
{
    int i = 0;

    while (i < n) {
        destination[i] = source[i];

        i++;
    }

    return destination;
}

void* memset(char* destination, int val, int n)
{
    int i = 0;

    while (i < n) {
        destination[i] = val;

        i++;
    }

    return destination;
}
