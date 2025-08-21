#include <string.h>
#include <stdlib.h>

#include "hashmap.h"
#include "semantic.h"

unsigned long hash(char *key) {
    unsigned long hash = 5381;
    char c;
    while((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASHMAP_TABLE_SIZE;
}

symbol_t *map_get(map_t *map, char *key) {
    unsigned long idx = hash(key);
    struct node *n = map->table[idx];

    if(n == NULL) return NULL;
    else {
        struct node *current = n;
        while(current != NULL) {
            if(strcmp(current->key, key) == 0) return current->value;
            current = current->next;
        }
    }

    return NULL;
}

int map_add(map_t *map, char *key, symbol_t *value) {
    unsigned long idx = hash(key);
    struct node *n = map->table[idx];

    if(n == NULL) {
        n = malloc(sizeof(struct node));
        n->key = key; // no strdup cuz i dont have to
        n->value = value;
        n->next = NULL;

        map->table[idx] = n;
    }

    else {
        struct node *current = n;
        while(current->next != NULL) {
            if(strcmp(current->key, key) == 0) return -1;
            current = current->next;
        }
        struct node *new = malloc(sizeof(struct node));
        new->key = key;
        new->value = value;
        new->next = NULL;
        current->next = new;
    }

    return 0;
}



map_t *get_map() {
    map_t *map = malloc(sizeof(map_t));
    memset(map, 0, sizeof(map_t));

    return map;
}
