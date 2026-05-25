/**
 * MERO Compiler - String Interning Pool
 * Deduplicates strings for fast comparison (pointer equality).
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_STRING_POOL_H
#define MERO_STRING_POOL_H

#include "mero/arena.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MERO_STRING_POOL_INITIAL_CAPACITY 256

typedef struct MeroInternedString {
    const char *data;
    size_t length;
    uint32_t hash;
} MeroInternedString;

typedef struct MeroStringPool {
    MeroInternedString *entries;
    size_t capacity;
    size_t count;
    size_t mask;
    MeroArena arena;
} MeroStringPool;

/* Initialize string pool */
void mero_string_pool_init(MeroStringPool *pool);

/* Destroy string pool and free all memory */
void mero_string_pool_destroy(MeroStringPool *pool);

/* Intern a null-terminated string; returns pointer-stable interned string */
const char *mero_string_pool_intern(MeroStringPool *pool, const char *str);

/* Intern a string with explicit length */
const char *mero_string_pool_intern_n(MeroStringPool *pool, const char *str, size_t len);

/* Check if a string is already interned (returns NULL if not found) */
const char *mero_string_pool_lookup(const MeroStringPool *pool, const char *str);

/* Get number of interned strings */
size_t mero_string_pool_count(const MeroStringPool *pool);

/* Hash function (FNV-1a) */
uint32_t mero_hash_string(const char *str, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* MERO_STRING_POOL_H */
