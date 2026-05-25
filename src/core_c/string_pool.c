/**
 * MERO Compiler - String Interning Pool Implementation
 * Developer: MERO:TG@QP4RM
 */
#include "mero/string_pool.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

uint32_t mero_hash_string(const char *str, size_t len) {
    uint32_t hash = 2166136261u;  /* FNV offset basis */
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint8_t)str[i];
        hash *= 16777619u;  /* FNV prime */
    }
    return hash;
}

static void string_pool_grow(MeroStringPool *pool) {
    size_t new_capacity = pool->capacity * 2;
    MeroInternedString *new_entries = (MeroInternedString *)calloc(
        new_capacity, sizeof(MeroInternedString)
    );
    if (!new_entries) {
        fprintf(stderr, "MERO: Fatal - string pool grow OOM\n");
        abort();
    }

    size_t new_mask = new_capacity - 1;
    for (size_t i = 0; i < pool->capacity; i++) {
        if (pool->entries[i].data == NULL) continue;

        uint32_t idx = pool->entries[i].hash & new_mask;
        while (new_entries[idx].data != NULL) {
            idx = (idx + 1) & new_mask;
        }
        new_entries[idx] = pool->entries[i];
    }

    free(pool->entries);
    pool->entries = new_entries;
    pool->capacity = new_capacity;
    pool->mask = new_mask;
}

void mero_string_pool_init(MeroStringPool *pool) {
    pool->capacity = MERO_STRING_POOL_INITIAL_CAPACITY;
    pool->count = 0;
    pool->mask = pool->capacity - 1;
    pool->entries = (MeroInternedString *)calloc(
        pool->capacity, sizeof(MeroInternedString)
    );
    if (!pool->entries) {
        fprintf(stderr, "MERO: Fatal - string pool init OOM\n");
        abort();
    }
    mero_arena_init(&pool->arena, 0);
}

void mero_string_pool_destroy(MeroStringPool *pool) {
    free(pool->entries);
    pool->entries = NULL;
    pool->capacity = 0;
    pool->count = 0;
    mero_arena_destroy(&pool->arena);
}

const char *mero_string_pool_intern_n(MeroStringPool *pool, const char *str, size_t len) {
    if (pool->count * 4 >= pool->capacity * 3) {
        string_pool_grow(pool);
    }

    uint32_t hash = mero_hash_string(str, len);
    uint32_t idx = hash & pool->mask;

    while (pool->entries[idx].data != NULL) {
        if (pool->entries[idx].hash == hash &&
            pool->entries[idx].length == len &&
            memcmp(pool->entries[idx].data, str, len) == 0) {
            return pool->entries[idx].data;
        }
        idx = (idx + 1) & pool->mask;
    }

    char *interned = mero_arena_strndup(&pool->arena, str, len);
    pool->entries[idx].data = interned;
    pool->entries[idx].length = len;
    pool->entries[idx].hash = hash;
    pool->count++;

    return interned;
}

const char *mero_string_pool_intern(MeroStringPool *pool, const char *str) {
    return mero_string_pool_intern_n(pool, str, strlen(str));
}

const char *mero_string_pool_lookup(const MeroStringPool *pool, const char *str) {
    size_t len = strlen(str);
    uint32_t hash = mero_hash_string(str, len);
    uint32_t idx = hash & pool->mask;

    while (pool->entries[idx].data != NULL) {
        if (pool->entries[idx].hash == hash &&
            pool->entries[idx].length == len &&
            memcmp(pool->entries[idx].data, str, len) == 0) {
            return pool->entries[idx].data;
        }
        idx = (idx + 1) & pool->mask;
    }
    return NULL;
}

size_t mero_string_pool_count(const MeroStringPool *pool) {
    return pool->count;
}
