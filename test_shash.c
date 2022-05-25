
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <err.h>

#include "shash.h"

typedef struct _hash_item {
    char                        k[10];
    char                        v[10];
    TAILQ_ENTRY(_hash_item)     entries;
} hash_item; 

typedef SHASH_HEAD(_hash_t, _hash_item) hash_t;
SHASH_GENERATE(ht, _hash_t, _hash_item, entries);

uint64_t hash_f(hash_item *i) {
    // FNV64A hash
    uint64_t hash = 0xcbf29ce484222325;
    char *v = i->k;
    char c;
    while ((c = *(v++))) {
        hash = (hash ^ c) * 0x100000001b3;
    }
    return hash;
}

int compare_f(hash_item *i, hash_item *j) {
    return strcmp(i->k, j->k);
}

void free_f(hash_item *i) {
    free(i);
}

void print_hash_item(hash_item *i) {
    if (i) 
        printf(">>> %s = %s\n", i->k, i->v);
    else
        printf(">>> Not Found\n");
}

int main(int argc, char **argv) {

    // Init hash table
    hash_t table = {0};
    if (ht_init(&table, 5000, hash_f, compare_f, free_f)) {
        err(1,"ht_init");
    }

    // Add items
    for (size_t i = 0; i < 10000; ++i) {
        hash_item *hi = calloc(1,sizeof(hash_item));
        snprintf(hi->k,sizeof(hi->k),"%09zu",i);
        snprintf(hi->v,sizeof(hi->v),"%09zu",999999-i);
        ht_set(&table,hi);
    }

    // Get item
    hash_item k = { .k = "000000099" };
    hash_item *v;
    ht_get(&table, &k, &v);
    print_hash_item(v);

    // Delete item
    ht_del(&table, &k);
    ht_get(&table, &k, &v);
    print_hash_item(v);

    // Duplicate keys
    hash_item *d1 = calloc(1,sizeof(hash_item));
    hash_item *d2 = calloc(1,sizeof(hash_item));

    snprintf(d1->k,sizeof(d1->k),"abcdef");
    snprintf(d1->v,sizeof(d1->v),"xxxxxx");
    snprintf(d2->k,sizeof(d2->k),"abcdef");
    snprintf(d2->v,sizeof(d2->v),"yyyyyy");

    ht_set(&table,d1);
    ht_set(&table,d2);
    ht_get(&table, d2, &v);
    print_hash_item(v);

    // Non existent key
    snprintf(k.k,sizeof(k.k),"xxxxxx");
    ht_get(&table, &k, &v);
    print_hash_item(v);

    ht_free(&table);

    return 0;

}
