#include <stdlib.h>

#include "lexer.h"
#include "trie.h"

int trie_index(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= 'A' && c <= 'Z') return 26 + (c - 'A');
    if (c >= '0' && c <= '9') return 52 + (c - '0');
    if (c == '_') return 62;
    return -1;
}

void trie_insert(struct trie *t, char *s, token_type_t type) {
    if(t->children == NULL) {
        t->children = calloc(TRIE_CHILDERN, sizeof(struct trie*));
    }

    while((*s) != '\0') {
        int idx = trie_index(*s);
        if(idx < 0) return;

        if(t->children[idx] == NULL) {
            t->children[idx] = calloc(1, sizeof(struct trie));
            t->children[idx]->children = calloc(TRIE_CHILDERN, sizeof(struct trie*));
        }

        t = t->children[idx];
        s++;
    }

    t->type = type;
}

token_type_t trie_get(struct trie *t, char *s) {
    while(*s != '\0') {
        int idx = trie_index(*s);
        if(idx < 0) return TOKEN_UNKNOWN;

        t = t->children[idx];
        if(t == NULL) return IDENTIFIER;
        s++;
    }

    return t->type;
}
