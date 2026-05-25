/**
 * MERO Compiler - Generic Hash Map Implementation
 * Developer: MERO:TG@QP4RM
 */
#include "mero/hash_map.h"
#include "mero/string_pool.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void map_grow(MeroHashMap *map);

void mero_map_init(MeroHashMap *map) {
    map->capacity = MERO_MAP_INITIAL_CAPACITY;
    map->count = 0;
    map->mask = map->capacity - 1;
    map->tombstone_count = 0;
    map->entries = (MeroMapEntry *)calloc(map->capacity, sizeof(MeroMapEntry));
    if (!map->entries) {
        fprintf(stderr, "MERO: Fatal - hash map init OOM\n");
        abort();
    }
}

void mero_map_destroy(MeroHashMap *map, MeroMapFreeFn free_fn) {
    if (free_fn) {
        for (size_t i = 0; i < map->capacity; i++) {
            if (map->entries[i].occupied && !map->entries[i].tombstone) {
                free_fn(map->entries[i].value);
            }
        }
    }
    free(map->entries);
    map->entries = NULL;
    map->capacity = 0;
    map->count = 0;
}

static uint32_t map_find_slot(const MeroHashMap *map, const char *key, uint32_t hash) {
    uint32_t idx = hash & map->mask;
    uint32_t first_tombstone = UINT32_MAX;

    while (1) {
        MeroMapEntry *entry = &map->entries[idx];
        if (!entry->occupied) {
            return (first_tombstone != UINT32_MAX) ? first_tombstone : idx;
        }
        if (entry->tombstone) {
            if (first_tombstone == UINT32_MAX) {
                first_tombstone = idx;
            }
        } else if (entry->key_hash == hash && strcmp(entry->key, key) == 0) {
            return idx;
        }
        idx = (idx + 1) & map->mask;
    }
}

static void map_grow(MeroHashMap *map) {
    size_t new_capacity = map->capacity * 2;
    MeroMapEntry *new_entries = (MeroMapEntry *)calloc(new_capacity, sizeof(MeroMapEntry));
    if (!new_entries) {
        fprintf(stderr, "MERO: Fatal - hash map grow OOM\n");
        abort();
    }

    size_t new_mask = new_capacity - 1;
    for (size_t i = 0; i < map->capacity; i++) {
        MeroMapEntry *entry = &map->entries[i];
        if (!entry->occupied || entry->tombstone) continue;

        uint32_t idx = entry->key_hash & new_mask;
        while (new_entries[idx].occupied) {
            idx = (idx + 1) & new_mask;
        }
        new_entries[idx] = *entry;
    }

    free(map->entries);
    map->entries = new_entries;
    map->capacity = new_capacity;
    map->mask = new_mask;
    map->tombstone_count = 0;
}

void *mero_map_set(MeroHashMap *map, const char *key, void *value) {
    if ((map->count + map->tombstone_count + 1) * 100 > map->capacity * 75) {
        map_grow(map);
    }

    size_t len = strlen(key);
    uint32_t hash = mero_hash_string(key, len);
    uint32_t idx = map_find_slot(map, key, hash);
    MeroMapEntry *entry = &map->entries[idx];

    if (entry->occupied && !entry->tombstone) {
        void *old = entry->value;
        entry->value = value;
        return old;
    }

    if (entry->tombstone) {
        map->tombstone_count--;
    }

    entry->key = key;
    entry->key_hash = hash;
    entry->value = value;
    entry->occupied = true;
    entry->tombstone = false;
    map->count++;
    return NULL;
}

void *mero_map_get(const MeroHashMap *map, const char *key) {
    if (map->count == 0) return NULL;

    size_t len = strlen(key);
    uint32_t hash = mero_hash_string(key, len);
    uint32_t idx = hash & map->mask;

    while (1) {
        MeroMapEntry *entry = &map->entries[idx];
        if (!entry->occupied) return NULL;
        if (!entry->tombstone && entry->key_hash == hash &&
            strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        idx = (idx + 1) & map->mask;
    }
}

bool mero_map_has(const MeroHashMap *map, const char *key) {
    return mero_map_get(map, key) != NULL;
}

void *mero_map_remove(MeroHashMap *map, const char *key) {
    if (map->count == 0) return NULL;

    size_t len = strlen(key);
    uint32_t hash = mero_hash_string(key, len);
    uint32_t idx = hash & map->mask;

    while (1) {
        MeroMapEntry *entry = &map->entries[idx];
        if (!entry->occupied) return NULL;
        if (!entry->tombstone && entry->key_hash == hash &&
            strcmp(entry->key, key) == 0) {
            void *value = entry->value;
            entry->tombstone = true;
            entry->value = NULL;
            map->count--;
            map->tombstone_count++;
            return value;
        }
        idx = (idx + 1) & map->mask;
    }
}

size_t mero_map_count(const MeroHashMap *map) {
    return map->count;
}

void mero_map_clear(MeroHashMap *map, MeroMapFreeFn free_fn) {
    if (free_fn) {
        for (size_t i = 0; i < map->capacity; i++) {
            if (map->entries[i].occupied && !map->entries[i].tombstone) {
                free_fn(map->entries[i].value);
            }
        }
    }
    memset(map->entries, 0, map->capacity * sizeof(MeroMapEntry));
    map->count = 0;
    map->tombstone_count = 0;
}

MeroMapIterator mero_map_iter(const MeroHashMap *map) {
    MeroMapIterator iter;
    iter.map = map;
    iter.index = 0;
    return iter;
}

bool mero_map_iter_next(MeroMapIterator *iter, const char **key, void **value) {
    while (iter->index < iter->map->capacity) {
        const MeroMapEntry *entry = &iter->map->entries[iter->index];
        iter->index++;
        if (entry->occupied && !entry->tombstone) {
            if (key) *key = entry->key;
            if (value) *value = entry->value;
            return true;
        }
    }
    return false;
}
