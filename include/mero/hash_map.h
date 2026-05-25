/**
 * MERO Compiler - Generic Hash Map
 * Open-addressing hash map with linear probing.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_HASH_MAP_H
#define MERO_HASH_MAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MERO_MAP_INITIAL_CAPACITY 16
#define MERO_MAP_LOAD_FACTOR 0.75

typedef struct MeroMapEntry {
    const char *key;
    uint32_t key_hash;
    void *value;
    bool occupied;
    bool tombstone;
} MeroMapEntry;

typedef struct MeroHashMap {
    MeroMapEntry *entries;
    size_t capacity;
    size_t count;
    size_t mask;
    size_t tombstone_count;
} MeroHashMap;

typedef void (*MeroMapFreeFn)(void *value);

/* Initialize hash map */
void mero_map_init(MeroHashMap *map);

/* Destroy hash map (optionally free values with callback) */
void mero_map_destroy(MeroHashMap *map, MeroMapFreeFn free_fn);

/* Insert or update a key-value pair. Returns previous value or NULL */
void *mero_map_set(MeroHashMap *map, const char *key, void *value);

/* Get value by key (NULL if not found) */
void *mero_map_get(const MeroHashMap *map, const char *key);

/* Check if key exists */
bool mero_map_has(const MeroHashMap *map, const char *key);

/* Remove key. Returns removed value or NULL */
void *mero_map_remove(MeroHashMap *map, const char *key);

/* Get number of entries */
size_t mero_map_count(const MeroHashMap *map);

/* Clear all entries */
void mero_map_clear(MeroHashMap *map, MeroMapFreeFn free_fn);

/* Iterator */
typedef struct MeroMapIterator {
    const MeroHashMap *map;
    size_t index;
} MeroMapIterator;

MeroMapIterator mero_map_iter(const MeroHashMap *map);
bool mero_map_iter_next(MeroMapIterator *iter, const char **key, void **value);

#ifdef __cplusplus
}
#endif

#endif /* MERO_HASH_MAP_H */
