#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "arena.h"
#include "fnv1a.h"

typedef struct {
    char *key;
    void *value;
    bool valid;
} Entry;

typedef struct {
    size_t items;
    size_t size;
    Arena arena;
    Entry *data;
} HashTable;


#define ht_set(ht, key, value) (_ht_set((ht), (key), (void *) &(value), sizeof(value)))

char *alloc(Arena *arena, void *s, size_t n);
uint64_t hash(char *s);
int find_slot(HashTable *ht, char *key);
void rehash(HashTable **ht);
size_t _ht_set(HashTable *ht, char *key, void *value, size_t n);
int is_valid(HashTable *ht, size_t i);

void *ht_lookup(HashTable *ht, char *key);
int *ht_remove(HashTable *ht, char *key);
int ht_lenght(HashTable ht);
void ht_destroy(HashTable *ht);


char *alloc(Arena *arena, void *s, size_t n)
{
    char *p = arena_allocate(arena, n);
    memcpy(p, s, n);
    return p;
}

uint64_t hash(char *s)
{
    return fnv1a(s, strlen(s));
}

int find_slot(HashTable *ht, char *key)
{
    if (!ht->data)
        goto ERROR;

    uint64_t h = hash(key);
    size_t pos = h % ht->size;

    size_t i = pos;
    while (is_valid(ht, pos) && strcmp(key, ht->data[pos].key)) {
        pos = (pos + 1) % ht->size;
        if (pos == i) {
            goto ERROR;
        }
    }

    return pos;

ERROR:
    return -1;
}

void rehash(HashTable **ht)
{
    HashTable *new = calloc(1, sizeof(HashTable));
    new->size = (*ht)->size ? (*ht)->size*2 : 256;
    new->data = calloc(new->size, sizeof(Entry));

    if (!new->data) {
        fprintf(stderr, "Couldn't find memory.\n");
        exit(-1);
    }

    for (size_t i = 0; i < (*ht)->size; i++)
        if (is_valid(*ht, i))
            _ht_set(new, (*ht)->data[i].key, (*ht)->data[i].value, sizeof(void*));
    ht_destroy(*ht);
    **ht = *new;
}


size_t _ht_set(HashTable *ht, char *key, void *value, size_t n)
{
    if (ht->items >= ((size_t) ht->size * 0.5)) {
        rehash(&ht);
    }

    if (!ht_lookup(ht, key)) {
        key = alloc(&ht->arena, key, strlen(key)+1);
        ht->items++;
    }

    value = alloc(&ht->arena, value, n);
    size_t pos = find_slot(ht, key);
    ht->data[pos] = (Entry) { key, value, true };
    return pos;
}

void *ht_lookup(HashTable *ht, char *key)
{
    int i = find_slot(ht, key);
    if (i >= 0 && is_valid(ht, i))
        return (void *) ht->data[i].value;
    else
        return (void *) NULL;
}

int is_valid(HashTable *ht, size_t i)
{
    return ht->data[i].valid;
}

int *ht_remove(HashTable *ht, char *key)
{
    int i = find_slot(ht, key);
    if (i < 0 || !is_valid(ht, i))
        return 0;

    ht->data[i].valid = false;
    int j = i;

    while (1) {
        j = (j+1) % ht->size;

        if (!is_valid(ht, j))
            break;

        int k = hash(ht->data[j].key) % ht->size;

        if (i <= j) {
            if (i < k && k <= j)
                continue;
        } else {
            if (k <= j || i < k)
                    continue;
        }

        ht->data[i].valid = true;
        ht->data[i].key = ht->data[j].key;
        ht->data[i].value = ht->data[j].value;
        ht->data[j].valid = false;
        i = j;
    }

    ht->items--;
    return 0;
}

void ht_destroy(HashTable *ht)
{
    arena_destroy(&ht->arena);
    free(ht->data);
}

size_t ht_get_nitems(HashTable ht)
{
    return ht.items;
}

size_t ht_get_size(HashTable ht)
{
    return ht.size;
}

#endif
