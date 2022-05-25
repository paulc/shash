#ifndef _SHASH_H
#define _SHASH_H

#include <sys/queue.h>

#define SHASH_HEAD(NAME,TYPE)                               \
    struct NAME {                                           \
        size_t n;                                           \
        uint64_t (*hash_f)(struct TYPE *i);                 \
        int (*compare_f)(struct TYPE *i,struct TYPE *j);    \
        void (*free_f)(struct TYPE *i);                     \
        TAILQ_HEAD(,TYPE) *buckets;                         \
}

#define SHASH_GENERATE(PREFIX,HEAD,TYPE,NAME)               \
int PREFIX##_init(struct HEAD *table, size_t n,             \
        uint64_t (*hash_f)(struct TYPE *i),                 \
        int (*compare_f)(struct TYPE *i,struct TYPE *j),    \
        void (*free_f)(struct TYPE *i)) {                   \
    SHASH_INIT(table, n, hash_f, compare_f, free_f);        \
    return SHASH_INIT_ERROR(table);                         \
}                                                           \
void PREFIX##_free(struct HEAD *table) {                    \
    SHASH_FREE(table, TYPE, NAME);                          \
}                                                           \
int PREFIX##_get(struct HEAD *table, struct TYPE *k, struct TYPE **v) { \
    SHASH_GET(table, TYPE, k, *v, NAME);                    \
    return (*v != NULL);                                    \
}                                                           \
void PREFIX##_set(struct HEAD *table, struct TYPE *v) {     \
    SHASH_SET(table, TYPE, v, NAME);                        \
}                                                           \
void PREFIX##_del(struct HEAD *table, struct TYPE *k) {     \
    SHASH_DEL(table, TYPE, k, NAME);                        \
}                                                           \
void PREFIX##_apply(struct HEAD *table, void (*map_f)(struct TYPE *i, void *data), void *data) { \
    struct TYPE *ip;                                        \
    SHASH_FOREACH(ip, table, NAME) map_f(ip,data);          \
}

#define SHASH_INIT(HEAD,N,HASH_F,COMPARE_F,FREE_F)          \
    do {                                                    \
        (HEAD)->buckets = calloc(N,sizeof(*((HEAD)->buckets))); \
        (HEAD)->n = N;                                      \
        (HEAD)->hash_f = HASH_F;                            \
        (HEAD)->compare_f = COMPARE_F;                      \
        (HEAD)->free_f = FREE_F;                            \
        if ((HEAD)->buckets)                                \
            for (int i = 0; i < N; ++i)                     \
                TAILQ_INIT(((HEAD)->buckets)+i);            \
    } while(0)  

#define SHASH_INIT_ERROR(HEAD)                              \
    ((HEAD)->buckets == NULL)

#define SHASH_FREE(HEAD,TYPE,NAME)                          \
    do {                                                    \
        if ((HEAD)->free_f) {                               \
            struct TYPE *ip;                                \
            for (size_t i = 0; i < (HEAD)->n; ++i) {        \
                while (!TAILQ_EMPTY(((HEAD)->buckets)+i)) { \
                    ip = TAILQ_FIRST(((HEAD)->buckets)+i);  \
                    TAILQ_REMOVE(((HEAD)->buckets)+i,ip,NAME); \
                    (HEAD)->free_f(ip);                     \
                }                                           \
            }                                               \
        }                                                   \
        free((HEAD)->buckets);                              \
        (HEAD)->buckets = NULL;                             \
    } while(0)

#define SHASH_SET(HEAD,TYPE,VALUE,NAME)                     \
    do {                                                    \
        size_t hv = ((HEAD)->hash_f)(VALUE) % (HEAD)->n;    \
        if (!TAILQ_EMPTY(((HEAD)->buckets)+hv)) {           \
            struct TYPE *ip;                                \
            TAILQ_FOREACH(ip,((HEAD)->buckets)+hv,NAME) {   \
                if ((HEAD)->compare_f(ip,VALUE) == 0) {     \
                    TAILQ_REMOVE(((HEAD)->buckets)+hv,ip,NAME); \
                    if ((HEAD)->free_f) (HEAD)->free_f(ip); \
                    break;                                  \
                }                                           \
            }                                               \
        }                                                   \
        TAILQ_INSERT_TAIL(((HEAD)->buckets)+hv,VALUE,NAME); \
    } while(0);

#define SHASH_GET(HEAD,TYPE,SEARCH_V,RETURN_V,NAME)         \
    do {                                                    \
        size_t hv = ((HEAD)->hash_f)(SEARCH_V) % (HEAD)->n; \
        struct TYPE *ip;                                    \
        RETURN_V = NULL;                                    \
        TAILQ_FOREACH(ip,((HEAD)->buckets)+hv,NAME) {       \
            if ((HEAD)->compare_f(ip,SEARCH_V) == 0) {      \
                RETURN_V = ip;                              \
                break;                                      \
            }                                               \
        }                                                   \
    } while(0);

#define SHASH_DEL(HEAD,TYPE,VALUE,NAME)                     \
    do {                                                    \
        size_t hv = ((HEAD)->hash_f)(VALUE) % (HEAD)->n;    \
        if (!TAILQ_EMPTY(((HEAD)->buckets)+hv)) {           \
            struct TYPE *ip;                                \
            TAILQ_FOREACH(ip,((HEAD)->buckets)+hv,NAME) {   \
                if ((HEAD)->compare_f(ip,VALUE) == 0) {     \
                    TAILQ_REMOVE(((HEAD)->buckets)+hv,ip,NAME); \
                    if ((HEAD)->free_f) (HEAD)->free_f(ip); \
                    break;                                  \
                }                                           \
            }                                               \
        }                                                   \
    } while(0);

#define SHASH_FOREACH(VAR,HEAD,NAME)                        \
    for (size_t i = 0; i < (HEAD)->n; ++i)                  \
        TAILQ_FOREACH(VAR,((HEAD)->buckets)+i,NAME)

#define SHASH_STATS(HEAD,TYPE,NAME)                         \
    do {                                                    \
        for (size_t i = 0; i < (HEAD)->n; ++i) {            \
            size_t count = 0;                               \
            struct TYPE *ip;                                \
            TAILQ_FOREACH(ip,((HEAD)->buckets)+i,NAME) ++count; \
            printf("Bucket %zu: %zu\n",i,count);            \
        }                                                   \
    } while(0);

#endif // _SHASH_H

