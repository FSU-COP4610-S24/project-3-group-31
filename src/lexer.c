#include "lexer.h"
#include "info.h"
#include "navigate.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void lexer(FAT32FileSystem* fs)
{	
	char path[256] = "fat32.img";
	bool valid;
	while (1) {
		// FIXME: Find a way to show path from inside filesystem
		printf("%s/> ", path);
		valid = false;	// Miscellaneous bool usage

		char *input = get_input();

		tokenlist *tokens = get_tokens(input);

        // Everything is an internal command!!
		if (tokens->size > 0){
			// conditions for internal commands
            if (strcmp(tokens->items[0], "exit") == 0){
				break;
            }
            else if (strcmp(tokens->items[0], "info") == 0){
                info(fs);
            }
			else if(strcmp(tokens->items[0], "ls") == 0){
				ls(fs);
			}
			else if(strcmp(tokens->items[0], "cd") == 0){
				if (tokens->size == 1)
					printf("Provide a directory to change to.\n");
				else {
					if (strcmp(tokens->items[1],"..") == 0) {
						if (goToParent(fs))
							upOneDir(path);
						else
							printf("Failed to move up a directory, you must already be at root.\n");
					}
					else {
						if (cd(fs, tokens->items[1])) {
							strcat(path, "/");
							strcat(path, tokens->items[1]);
						}
					}
				}
			}
			else if (strcmp(tokens->items[0], "mkdir") == 0) {
				if (tokens->size < 2) {
					printf("Error: Missing directory name.\n");
				}
				else {
					mkdir(fs, tokens->items[1]);
					printf("Directory '%s' created successfully.\n", tokens->items[1]);
				}
			}
			else if (strcmp(tokens->items[0], "creat") == 0) {
				if (tokens->size < 2) {
					printf("Error: Missing file name.\n");
				}
				else {
					if(createFile(fs, tokens->items[1])){
						printf("Error: Directory is full, no free entries found.\n");
					}
					else {
						printf("File '%s' created successfully.\n", tokens->items[1]);
					}
				}
			}
			else{
				printf("Command not '%s' found.\n", tokens->items[0]);
			}
        }
		free(input);
		free_tokens(tokens);
	}
}

char *get_input(void) {
	char *buffer = NULL;
	int bufsize = 0;
	char line[5];
	while (fgets(line, 5, stdin) != NULL)
	{
		int addby = 0;
		char *newln = strchr(line, '\n');
		if (newln != NULL)
			addby = newln - line;
		else
			addby = 5 - 1;
		buffer = (char *)realloc(buffer, bufsize + addby);
		memcpy(&buffer[bufsize], line, addby);
		bufsize += addby;
		if (newln != NULL)
			break;
	}
	buffer = (char *)realloc(buffer, bufsize + 1);
	buffer[bufsize] = 0;
	return buffer;
}

tokenlist *new_tokenlist(void) {
	tokenlist *tokens = (tokenlist *)malloc(sizeof(tokenlist));
	tokens->size = 0;
	tokens->items = (char **)malloc(sizeof(char *));
	tokens->items[0] = NULL; /* make NULL terminated */
	return tokens;
}

void add_token(tokenlist *tokens, char *item) {
	int i = tokens->size;

	tokens->items = (char **)realloc(tokens->items, (i + 2) * sizeof(char *));
	tokens->items[i] = (char *)malloc(strlen(item) + 1);
	tokens->items[i + 1] = NULL;
	strcpy(tokens->items[i], item);

	tokens->size += 1;
}

tokenlist *get_tokens(char *input) {
	char *buf = (char *)malloc(strlen(input) + 1);
	strcpy(buf, input);
	tokenlist *tokens = new_tokenlist();
	char *tok = strtok(buf, " ");
	while (tok != NULL)
	{
		add_token(tokens, tok);
		tok = strtok(NULL, " ");
	}
	free(buf);
	return tokens;
}

void free_tokens(tokenlist *tokens) {
	for (int i = 0; i < tokens->size; i++)
		free(tokens->items[i]);
	free(tokens->items);
	free(tokens);
}

void getImageName(char* filename, char* buffer) {
	buffer = strchr(filename, '/');
	if(buffer != NULL)
		buffer++;
	else
		strcpy(buffer, filename);
}

void upOneDir(char* path) {
	char* lastSlash = path;
	char* nextSlash;

	do {
	nextSlash = lastSlash;
	lastSlash += sizeof(char);
	lastSlash = strchr(lastSlash, '/');
	} while (lastSlash != NULL);
	
	*nextSlash ='\0';
}