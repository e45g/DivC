#ifndef _HASHMAP_H
#define _HASHMAP_H

#include "semantic.h"

#define HASHMAP_TABLE_SIZE 1024

struct node {
    char *key;
    symbol_t *value;
    struct node *next;
};

typedef struct map {
    struct node *table[HASHMAP_TABLE_SIZE];
} map_t;

map_t *get_map();
int map_add(map_t *map, char *key, symbol_t *value);
symbol_t *map_get(map_t *map, char *key);

#endif
