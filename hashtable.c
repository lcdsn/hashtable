#include <stdio.h>
#include <stdlib.h>

#include "hashtable.h"

#define MIN(x,y) (((x)<(y))?(x):(y))

int compar(const void *e1, const void *e2)
{
    uint64_t v1 =  *((uint64_t *) ((Entry*) e1)->value);
    uint64_t v2 =  *((uint64_t *) ((Entry*) e2)->value);
    return (int) v2 - v1;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s file num\n", argv[0]);
        return -1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "Couldn't open %s\n", argv[1]);
        return -1;
    }

    // Get the size of the file
    fseek(fp, 0L, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    // Allocate enough memory to copy the contents
    Arena arena = {0};
    char *s = arena_allocate(&arena, size);

    // Copy the contents
    fread(s, size, 1, fp);

    // Create the hash table
    HashTable ht = {0};
    ht.arena = &arena;

    // tokenize and insert into the hash table
    char *delim = " \n\t\r";
    char *token = strtok(s, delim);
    uint64_t count = 0;
    while (token) {
        void *v = ht_get(&ht, token);
        count = v ? *((uint64_t *) v) + 1 : 1;
        ht_set(&ht, token, count);
        token = strtok(NULL, delim);
    }

    // just for fun
    ht_remove(&ht, "lucas");


    // Allocate enough space for all hash table entries
    Arena words = {0};
    Entry *p = (Entry *) arena_allocate(&words, sizeof(Entry) * ht_get_nitems(ht));

    // Copy the entries
    Entry *q = p;
    for (size_t i = 0; i < ht_get_size(ht); i++) {
        if (ht.data[i].valid) {
            memcpy(q, &ht.data[i], sizeof(Entry));
            q++;
        }
    }

    // sort the words
    qsort(p, ht_get_nitems(ht), sizeof(Entry), compar);
    
    printf("number of words: %zu\n", ht_get_nitems(ht));
    uint64_t num = atoi(argv[2]);
    printf("Top-%lu words:\n", num);
    for (size_t i = 0; i < MIN(num, ht_get_nitems(ht)); i++) {
        printf("%s: %zu\n", (char *) p[i].key,  *((uint64_t *) p[i].value));
    }

    arena_destroy(&arena);
    arena_destroy(&words);
    ht_destroy(&ht);

    return 0;
}

