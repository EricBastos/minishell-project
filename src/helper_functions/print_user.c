#include <stdio.h>
#include <stdlib.h>
#include "../utils.h"

int main() {
    char* str = readString(stdin);
    printf("Print user: %s\n", str);
    free(str);
    return 0;

}