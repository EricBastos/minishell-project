#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    char str[255] = "Print argv:";
    for (int i=0; i<argc; i++) {
        strcat(str, " ");
        strcat(str, argv[i]);
    }
    printf("%s\n", str);
    return 0;

}