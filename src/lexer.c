#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "trie.h"

void lexer_push(token_t **n, token_type_t type, char *val) {
    (*n)->next = malloc(sizeof(token_t));
    (*n)->next->type = type;
    (*n)->next->value = val;
    (*n)->next->next = NULL;

    *n = (*n)->next;
}

token_t *lexer_parse(char *src) {
    struct trie keywords = {0};
    trie_insert(&keywords, "print", PRINT);
    trie_insert(&keywords, "i8", I8);
    trie_insert(&keywords, "i16", I16);
    trie_insert(&keywords, "i32", I32);
    trie_insert(&keywords, "int", I32);
    trie_insert(&keywords, "i64", I64);
    trie_insert(&keywords, "u8", U8);
    trie_insert(&keywords, "u16", U16);
    trie_insert(&keywords, "u32", U32);
    trie_insert(&keywords, "u64", U64);

    token_t *list = malloc(sizeof(token_t));
    token_t *head = list;
    memset(list, '\0', sizeof(token_t));

    size_t len = strlen(src);
    for(size_t i = 0; i < len; i++) {
        char current = src[i];
        char next;

        if(isspace(current)) continue;

        switch(current) {
            case '+': {
                next = src[i + 1];
                if(next == '+') {
                    lexer_push(&list, PLUS_PLUS, "++");
                    i++;
                }
                else if(next == '=') {
                    lexer_push(&list, PLUS_EQ, "+=");
                    i++;
                }
                else {
                    lexer_push(&list, PLUS, "+");
                }
                break;
            }
            case '-': {
                next = src[i + 1];
                if(next == '-') {
                    lexer_push(&list, MINUS_MINUS, "--");
                    i++;
                }
                else if(next == '>') {
                    lexer_push(&list, ARROW, "->");
                    i++;
                }
                else if(next == '=') {
                    lexer_push(&list, MINUS_EQ, "-=");
                    i++;
                }
                else {
                    lexer_push(&list, MINUS, "-");
                }
                break;
            }
            case '*': {
                next = src[i + 1];
                if(next == '=') {
                    lexer_push(&list, STAR_EQ, "*=");
                    i++;
                }
                else {
                    lexer_push(&list, STAR, "*");
                }
                break;
            }
            case '/': {
                next = src[i + 1];
                if(next == '=') {
                    lexer_push(&list, SLASH_EQ, "/=");
                    i++;
                }
                else {
                    lexer_push(&list, SLASH, "/");
                }
                break;
            }
            case '!': {
                next = src[i + 1];
                if(next == '=') {
                    lexer_push(&list, NOT_EQ, "!=");
                    i++;
                }
                else {
                    lexer_push(&list, NOT, "!");
                }
                break;
            }
            case ';': {
                lexer_push(&list, SEMICOLON, ";");
                break;
            }

            default: {
                if(isdigit(current)) {
                    size_t start = i;
                    while (isdigit(src[i])) {
                        i++;
                    }
                    size_t length = i - start;
                    char *val = malloc(length + 1);
                    strncpy(val, src + start, length);
                    val[length] = '\0';
                    lexer_push(&list, NUMBER, val);
                    i--;
                }
                else if(isalpha(current) || current == '_') {
                    size_t start = i;
                    while(isalpha(src[i]) || isdigit(src[i]) || src[i] == '_') {
                        i++;
                    }
                    size_t length = i - start;
                    char *val = malloc(length + 1);
                    strncpy(val, src+start, length);
                    val[length] = '\0';
                    i--;

                    token_type_t token_type = trie_get(&keywords, val);
                    lexer_push(&list, token_type, val);
                }
                else {
                    char *val = malloc(2);
                    val[0] = current;
                    val[1] = '\0';
                    lexer_push(&list, TOKEN_UNKNOWN, val);
                }
            }
        }
    }

    return head;
}
