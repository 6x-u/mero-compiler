/**
 * MERO Compiler - Arena Memory Allocator Implementation
 * Developer: MERO:TG@QP4RM
 */
#include "mero/arena.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static MeroArenaBlock *arena_new_block(size_t capacity) {
    MeroArenaBlock *block = (MeroArenaBlock *)malloc(
        sizeof(MeroArenaBlock) + capacity
    );
    if (!block) {
        fprintf(stderr, "MERO: Fatal - out of memory (requested %zu bytes)\n", capacity);
        abort();
    }
    block->next = NULL;
    block->capacity = capacity;
    block->used = 0;
    return block;
}

static size_t align_up(size_t value, size_t alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

void mero_arena_init(MeroArena *arena, size_t block_size) {
    if (block_size == 0) {
        block_size = MERO_ARENA_DEFAULT_BLOCK_SIZE;
    }
    arena->block_size = block_size;
    arena->head = arena_new_block(block_size);
    arena->current = arena->head;
    arena->total_allocated = sizeof(MeroArenaBlock) + block_size;
    arena->block_count = 1;
}

void *mero_arena_alloc(MeroArena *arena, size_t size) {
    return mero_arena_alloc_aligned(arena, size, MERO_ARENA_ALIGNMENT);
}

void *mero_arena_alloc_aligned(MeroArena *arena, size_t size, size_t alignment) {
    size_t aligned_used = align_up(arena->current->used, alignment);

    if (aligned_used + size > arena->current->capacity) {
        size_t new_cap = arena->block_size;
        if (size > new_cap) {
            new_cap = align_up(size + MERO_ARENA_ALIGNMENT, MERO_ARENA_ALIGNMENT);
        }
        MeroArenaBlock *new_block = arena_new_block(new_cap);
        arena->current->next = new_block;
        arena->current = new_block;
        arena->total_allocated += sizeof(MeroArenaBlock) + new_cap;
        arena->block_count++;
        aligned_used = 0;
    }

    void *ptr = arena->current->data + aligned_used;
    arena->current->used = aligned_used + size;
    return ptr;
}

void *mero_arena_calloc(MeroArena *arena, size_t count, size_t size) {
    size_t total = count * size;
    void *ptr = mero_arena_alloc(arena, total);
    memset(ptr, 0, total);
    return ptr;
}

char *mero_arena_strdup(MeroArena *arena, const char *str) {
    size_t len = strlen(str);
    char *copy = (char *)mero_arena_alloc(arena, len + 1);
    memcpy(copy, str, len + 1);
    return copy;
}

char *mero_arena_strndup(MeroArena *arena, const char *str, size_t n) {
    size_t len = 0;
    while (len < n && str[len]) len++;
    char *copy = (char *)mero_arena_alloc(arena, len + 1);
    memcpy(copy, str, len);
    copy[len] = '\0';
    return copy;
}

void mero_arena_reset(MeroArena *arena) {
    MeroArenaBlock *block = arena->head->next;
    while (block) {
        MeroArenaBlock *next = block->next;
        free(block);
        block = next;
    }
    arena->head->next = NULL;
    arena->head->used = 0;
    arena->current = arena->head;
    arena->total_allocated = sizeof(MeroArenaBlock) + arena->head->capacity;
    arena->block_count = 1;
}

void mero_arena_destroy(MeroArena *arena) {
    MeroArenaBlock *block = arena->head;
    while (block) {
        MeroArenaBlock *next = block->next;
        free(block);
        block = next;
    }
    arena->head = NULL;
    arena->current = NULL;
    arena->total_allocated = 0;
    arena->block_count = 0;
}

size_t mero_arena_total_size(const MeroArena *arena) {
    return arena->total_allocated;
}

size_t mero_arena_used_size(const MeroArena *arena) {
    size_t used = 0;
    MeroArenaBlock *block = arena->head;
    while (block) {
        used += block->used;
        block = block->next;
    }
    return used;
}
