#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "trie.h"


void lexer_push(token_t **n, token_type_t type, char *val, pos_t pos) {
    (*n)->next = malloc(sizeof(token_t));
    (*n)->next->type = type;
    (*n)->next->value = val;
    (*n)->next->next = NULL;
    (*n)->next->pos = pos;

    *n = (*n)->next;
}

token_t *lexer_parse(char *src) {
    struct trie keywords = {0};
    trie_insert(&keywords, "void", VOID);
    trie_insert(&keywords, "i8", I8);
    trie_insert(&keywords, "i16", I16);
    trie_insert(&keywords, "i32", I32);
    trie_insert(&keywords, "int", I32);
    trie_insert(&keywords, "i64", I64);
    trie_insert(&keywords, "u8", U8);
    trie_insert(&keywords, "u16", U16);
    trie_insert(&keywords, "u32", U32);
    trie_insert(&keywords, "u64", U64);
    trie_insert(&keywords, "unsigned", UNSIGNED);
    trie_insert(&keywords, "short", SHORT);
    trie_insert(&keywords, "long", LONG);
    trie_insert(&keywords, "return", RETURN);

    token_t *list = calloc(1, sizeof(token_t));
    token_t *head = list;

    pos_t pos = {1, 1};

    size_t len = strlen(src);
    size_t i = 0;
    size_t i2 = 0;

    while(i < len) {
        char current = src[i];
        char next = (i+1) < len ? src[i+1] : '\0';
        pos.column += i - i2;
        i2 = i;

        if(current == '\n'){
            pos.line++;
            pos.column = 0;
            i++;
            continue;
        }
        if(isspace(current)) {
            i++;
            continue;
        }

        switch(current) {
            case '+': {
                if(next == '+') {
                    lexer_push(&list, PLUS_PLUS, "++", pos);
                    i++;
                }
                else if(next == '=') {
                    lexer_push(&list, PLUS_EQ, "+=", pos);
                    i++;
                }
                else {
                    lexer_push(&list, PLUS, "+", pos);
                }
                break;
            }
            case '-': {
                if(next == '-') {
                    lexer_push(&list, MINUS_MINUS, "--", pos);
                    i++;
                }
                else if(next == '>') {
                    lexer_push(&list, ARROW, "->", pos);
                    i++;
                }
                else if(next == '=') {
                    lexer_push(&list, MINUS_EQ, "-=", pos);
                    i++;
                }
                else {
                    lexer_push(&list, MINUS, "-", pos);
                }
                break;
            }
            case '*': {
                if(next == '=') {
                    lexer_push(&list, STAR_EQ, "*=", pos);
                    i++;
                }
                else {
                    lexer_push(&list, STAR, "*", pos);
                }
                break;
            }
            case '/': {
                if(next == '=') {
                    lexer_push(&list, SLASH_EQ, "/=", pos);
                    i++;
                }
                else if(next == '/') {
                    lexer_push(&list, COMMENT, "//", pos);
                }
                else {
                    lexer_push(&list, SLASH, "/", pos);
                }
                break;
            }
            case '=': {
                if(next == '=') {
                    lexer_push(&list, EQUAL, "==", pos);
                    i++;
                }
                else {
                    lexer_push(&list, ASSIGN, "=", pos);
                }
                break;
            }

            case '!': {
                if(next == '=') {
                    lexer_push(&list, NOT_EQ, "!=", pos);
                    i++;
                }
                else {
                    lexer_push(&list, NOT, "!", pos);
                }
                break;
            }

            case '(': {
                lexer_push(&list, LEFT_PAREN, "(", pos);
                break;
            }

            case '{': {
                lexer_push(&list, LEFT_CURLY, "{", pos);
                break;
            }

            case '[': {
                lexer_push(&list, LEFT_SQUARE, "[", pos);
                break;
            }

            case ')': {
                lexer_push(&list, RIGHT_PAREN, ")", pos);
                break;
            }

            case '}': {
                lexer_push(&list, RIGHT_CURLY, "}", pos);
                break;
            }

            case ']': {
                lexer_push(&list, RIGHT_SQUARE, "]", pos);
                break;
            }

            case ';': {
                lexer_push(&list, SEMICOLON, ";", pos);
                break;
            }

            case ',': {
                lexer_push(&list, COMMA, ",", pos);
                break;
            }

            case '.': {
                lexer_push(&list, DOT, ".", pos);
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
                    lexer_push(&list, NUMBER, val, pos);
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
                    lexer_push(&list, token_type, val, pos);
                }
                else {
                    char *val = malloc(2);
                    val[0] = current;
                    val[1] = '\0';
                    lexer_push(&list, TOKEN_UNKNOWN, val, pos);
                }
            }
        }
        i++;
    }

    lexer_push(&list, TOKEN_EOF, "token-eof", pos);

    return head;
}
