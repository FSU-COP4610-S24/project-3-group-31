#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include "filesys.h"

typedef struct {
    char ** items;
    size_t size;
} tokenlist;

void lexer(FAT32FileSystem*);
char * get_input(void);
tokenlist * get_tokens(char *input);
tokenlist * new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);
void getImageName(char* filename, char* buffer);