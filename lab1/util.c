#include "util.h"

void tokenize(char* input, char* tokens[], int* token_count) {
    *token_count = 0;
    char *token = strtok(input, " \t\n\r");

    while (token != NULL) {
        tokens[(*token_count)++] = token;
        token = strtok(NULL, " \t\n\r");
    }
    
    tokens[*token_count] = NULL;
}
