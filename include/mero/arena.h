/**
 * MERO Compiler - Arena Memory Allocator
 * High-performance bump allocator for compiler data structures.
 * Allocations are freed all at once when the arena is destroyed.
 * Developer: MERO:TG@QP4RM
 */
#ifndef MERO_ARENA_H
#define MERO_ARENA_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MERO_ARENA_DEFAULT_BLOCK_SIZE (64 * 1024)  /* 64 KB */
#define MERO_ARENA_ALIGNMENT 8

typedef struct MeroArenaBlock {
    struct MeroArenaBlock *next;
    size_t capacity;
    size_t used;
    uint8_t data[];
} MeroArenaBlock;

typedef struct MeroArena {
    MeroArenaBlock *head;
    MeroArenaBlock *current;
    size_t block_size;
    size_t total_allocated;
    size_t block_count;
} MeroArena;

/* Initialize an arena with given block size (0 = default) */
void mero_arena_init(MeroArena *arena, size_t block_size);

/* Allocate memory from the arena (never returns NULL, aborts on OOM) */
void *mero_arena_alloc(MeroArena *arena, size_t size);

/* Allocate zero-initialized memory */
void *mero_arena_calloc(MeroArena *arena, size_t count, size_t size);

/* Allocate aligned memory */
void *mero_arena_alloc_aligned(MeroArena *arena, size_t size, size_t alignment);

/* Duplicate a string into the arena */
char *mero_arena_strdup(MeroArena *arena, const char *str);

/* Duplicate n bytes of a string into the arena */
char *mero_arena_strndup(MeroArena *arena, const char *str, size_t n);

/* Reset arena (free all blocks except the first, reset usage) */
void mero_arena_reset(MeroArena *arena);

/* Destroy arena and free all memory */
void mero_arena_destroy(MeroArena *arena);

/* Get total bytes allocated from system */
size_t mero_arena_total_size(const MeroArena *arena);

/* Get total bytes used by user allocations */
size_t mero_arena_used_size(const MeroArena *arena);

#ifdef __cplusplus
}
#endif

#endif /* MERO_ARENA_H */
