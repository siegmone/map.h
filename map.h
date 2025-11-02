#ifndef MAP_H
#define MAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "alloc.h"
#include "hash.h"
#include "types.h"

#define MAP_DEFAULT_CAPACITY 1
#define MAP_LOAD_HIGH        0.85
#define MAP_LOAD_LOW         0.15

typedef struct map_s map_t;

typedef hash_t (*map_hash_f)(map_t *, const void *);
typedef bool (*map_cmp_f)(map_t *, const void *, const void *);

typedef struct {
    hash_t hash;
    bool   used;
} map_entry_t;

struct map_s {
    allocator_t *allocator;

    map_hash_f f_hash;
    map_cmp_f  f_key_cmp;

    map_entry_t *entries;
    void        *keys, *values;
    size_t       key_size, value_size, used, capacity;

    bool lock_rehash;

    u8 prime;
};

/* hash functions */
hash_t map_hash_bytes(map_t *map, const void *p);

/* cmp  functions */
bool map_cmp_bytes(map_t *map, const void *p, const void *q);

/* map init/deinit */
void map_init(map_t       *map,
              allocator_t *allocator,
              size_t       key_size,
              size_t       value_size,
              map_hash_f   f_hash,
              map_cmp_f    f_key_cmp);
void map_deinit(map_t *map);

/* insert/lookup/deletion */
bool  map_insert(map_t *map, const void *key, const void *value);
void *map_get(map_t *map, const void *key);
bool  map_remove(map_t *map, const void *key);

/* linear probing function */
static inline hash_t map_probe(const map_t *map, hash_t hash) {
    return hash % map->capacity;
}

/* utility functions */
static inline void *map_key_at(map_t *map, size_t i) {
    return (u8 *)map->keys + (i * map->key_size);
}

static inline void *map_value_at(map_t *map, size_t i) {
    return (u8 *)map->values + (i * map->value_size);
}

#endif /* MAP_H */

#ifdef MAP_IMPL

/* planetmath.org/goodhashtableprimes */
static const u32 PRIMES[] = {
    11, 53, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317,
    196613, 393241, 786433, 1572869, 3145739, 6291469, 12582917, 25165843,
    50331653, 100663319, 201326611, 402653189, 805306457, 1610612741};

/* rehashing */
static void map_alloc(map_t *map) {
    const size_t old_capacity = map->capacity;
    map_entry_t *old_entries  = map->entries;
    void        *old_keys     = map->keys;
    void        *old_values   = map->values;

    map->capacity = PRIMES[map->prime];

    map->used = 0;

    map->entries = mem_alloc(map->allocator, map->capacity * sizeof(map_entry_t));
    map->keys    = mem_alloc(map->allocator, map->capacity * map->key_size);
    map->values  = mem_alloc(map->allocator, map->capacity * map->value_size);

    memset(map->entries, 0, map->capacity * sizeof(map_entry_t));

    if (old_entries && old_keys && old_values) {
        map->lock_rehash = true;
        for (size_t i = 0; i < old_capacity; i++) {
            if (!old_entries[i].used) {
                continue;
            }
            void *key   = (u8 *)old_keys + (i * map->key_size);
            void *value = (u8 *)old_values + (i * map->value_size);
            map_insert(map, key, value);
        }
        mem_free(map->allocator, old_entries);
        mem_free(map->allocator, old_keys);
        mem_free(map->allocator, old_values);
        map->lock_rehash = false;
    }
}

static bool map_rehash(map_t *map) {
    if (map->lock_rehash) {
        return false;
    }

    const size_t old_capacity = map->capacity;
    const float  load         = map->capacity == 0 ? 0.0f : (float)map->used / map->capacity;
    const bool low            = load<MAP_LOAD_LOW, high = load> MAP_LOAD_HIGH;

    if (!map->entries && !map->keys && !map->values) {
        map->prime = 0;
        map_alloc(map);
        return true;
    }

    if (low && map->prime != 0) {
        map->prime--;
        map_alloc(map);
    } else if (high && map->prime != ARRLEN(PRIMES) - 1) {
        map->prime++;
        map_alloc(map);
    }

    return true;
}

/* hash functions */
hash_t map_hash_bytes(map_t *map, const void *p) {
    return hash_add_bytes(0xDEADBEEF, p, map->key_size);
}

/* cmp  functions */
bool map_cmp_bytes(map_t *map, const void *p, const void *q) {
    return !memcmp(p, q, map->key_size);
}

/* map init/deinit */
void map_init(map_t       *map,
              allocator_t *allocator,
              size_t       key_size,
              size_t       value_size,
              map_hash_f   f_hash,
              map_cmp_f    f_key_cmp) {
    *map = (map_t){0};

    map->allocator  = allocator;
    map->key_size   = key_size;
    map->value_size = value_size;
    map->f_hash     = f_hash;
    map->f_key_cmp  = f_key_cmp;
    map->capacity   = MAP_DEFAULT_CAPACITY;

    map_alloc(map);
}

void map_deinit(map_t *map) {
    if (map->keys)
        mem_free(map->allocator, map->keys);
    if (map->values)
        mem_free(map->allocator, map->values);
    if (map->entries)
        mem_free(map->allocator, map->entries);

    map->used     = 0;
    map->capacity = 0;
    map->prime    = 0;
    map->keys     = NULL;
    map->values   = NULL;
    map->entries  = NULL;
}

/* insert/lookup/deletion */
bool map_insert(map_t *map, const void *key, const void *value) {
    float load      = map->capacity == 0 ? 0.0f : (float)map->used / map->capacity;
    bool  load_high = load > MAP_LOAD_HIGH;
    bool  load_low  = load < MAP_LOAD_LOW;
    if ((load_high || load_low) && !map->lock_rehash) {
        map_rehash(map);
    }

    hash_t h   = map->f_hash(map, key);
    size_t pos = map_probe(map, h);

    for (size_t i = 0; i < map->capacity; i++) {
        size_t       idx   = (pos + i) % map->capacity;
        map_entry_t *entry = &map->entries[idx];

        if (!entry->used) {
            /* new value */
            entry->used = true;
            entry->hash = h;
            memcpy(map_key_at(map, idx), key, map->key_size);
            memcpy(map_value_at(map, idx), value, map->value_size);
            map->used++;
            return true;
        }

        void *stored_key = map_key_at(map, idx);
        if (entry->hash == h && map->f_key_cmp(map, stored_key, key)) {
            /* update value */
            memcpy(map_value_at(map, idx), value, map->value_size);
            return true;
        }
    }
    return false;
}

void *map_get(map_t *map, const void *key) {
    hash_t h   = map->f_hash(map, key);
    size_t pos = map_probe(map, h);

    for (size_t i = 0; i < map->capacity; i++) {
        size_t       idx   = (pos + i) % map->capacity;
        map_entry_t *entry = &map->entries[idx];

        if (!entry->used)
            continue;

        if (entry->hash == h) {
            void *stored_key = map_key_at(map, idx);
            if (map->f_key_cmp(map, stored_key, key)) {
                return map_value_at(map, idx);
            }
        }
    }
    return NULL;
}

bool map_remove(map_t *map, const void *key) {
    hash_t h   = map->f_hash(map, key);
    size_t pos = map_probe(map, h);

    for (size_t i = 0; i < map->capacity; i++) {
        size_t       idx   = (pos + i) % map->capacity;
        map_entry_t *entry = &map->entries[idx];

        if (!entry->used)
            return false;

        void *stored_key = map_key_at(map, idx);
        if (entry->hash == h && map->f_key_cmp(map, stored_key, key)) {
            entry->used = false;
            map->used--;
            return true;
        }
    }

    return false;
}

#endif /* MAP_IMPL */
