#ifndef _HASHMAP_H
#define _HASHMAP_H

#define HASHMAP_TABLE_SIZE 1024

struct scope;
typedef struct symbol symbol_t;

unsigned long hash(char *key);
int map_add(struct scope *s, char *key, symbol_t *value);
symbol_t *map_get(struct scope *s, char *key);

#endif
