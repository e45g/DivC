#ifndef _TRIE_H
#define _TRIE_H

#include "lexer.h"

#define TRIE_CHILDERN 63

struct trie {
    struct trie **children;
    token_type_t type;
};

void trie_insert(struct trie *t, char *s, token_type_t type);
token_type_t trie_get(struct trie *t, char *s);

#endif
