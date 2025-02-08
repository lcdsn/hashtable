#ifndef _ARENA_H_
#define _ARENA_H_

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef ALIGN
#define ALIGN 1024*2
#endif

typedef struct region {
    struct region *next;
    char *limit;
    char *avail;
} Region;

typedef struct {
    Region *entry;
    Region *current;
} Arena;

char *arena_allocate(Arena *a, size_t n);
void arena_deallocate(Arena *a);
void arena_destroy(Arena *a);

char *arena_allocate(Arena *a, size_t n)
{
    Region **rp = &a->current;
    while(*rp && (n >= (size_t) ((*rp)->limit - (*rp)->avail)))
        rp = &(*rp)->next;

    if (!*rp) {
        int m = ((n + ALIGN - 1) & ~(ALIGN - 1));
        *rp = malloc(m);

        if (!*rp) {
            fprintf(stderr, "Couldn't find memory.\n");
            return NULL;
        }

        (*rp)->next = NULL;
        (*rp)->limit = (char *) (*rp) + m;
        (*rp)->avail = (char *) (*rp) + sizeof(Region);
        a->current = *rp;

       if (!a->entry)
            a->entry = *rp;
    }

    (*rp)->avail += n;
    return (*rp)->avail - n;
}

void arena_deallocate(Arena *a)
{
    for (Region *r = a->entry; r; r = r->next)
        r->avail = (char *) r + sizeof(Region);
    a->current = a->entry;
}

void arena_destroy(Arena *a)
{
    Region *rp = a->entry;
    while (rp) {
        Region *tmp = rp;
        rp = rp->next;
        free(tmp);
    }
    a->entry = NULL;
    a->current = NULL;
}

#endif
