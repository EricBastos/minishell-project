#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

char* readString(FILE* fp){

    char *str; // String buffer
    int ch; // Current char
    size_t len = 0;
    size_t currCapacity = 16;
    str = realloc(NULL, sizeof(*str)*currCapacity); // Realloc initial string space

    if(!str)return str; // Return if wasn't able to do so

    while((ch=fgetc(fp))!=EOF && ch != '\n'){ // Keep reading until EOF or \n
        str[len++]=ch; // Append char
        if(len==currCapacity){ // Check if used all allocated space
            currCapacity *= 2; // Double space
            str = realloc(str, sizeof(*str)*(currCapacity)); // Realloc
            if(!str)return str; // Return if error
        }
    }
    str[len++]='\0'; // C strings end with \0

    return realloc(str, sizeof(*str)*len); // Realloc to use the correct space
}

char* trimWhiteSpace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}