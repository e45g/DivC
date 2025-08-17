#include <stdio.h>
#include <string.h>
#include "lexer.h"

int main(void) {
    char SOURCE[] = "int x = 1 + 2;";
    
    token_t *token = lexer_parse(SOURCE);
    token_t *current = token->next;
    for(; current->next != NULL; current=current->next) {
        printf("%s\n", current->value);
    }

    return 0;
}
