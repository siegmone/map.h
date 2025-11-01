#ifndef HASH_H
#define HASH_H

#include <stddef.h>
#include <stdint.h>

typedef uint64_t hash_t;

/* hash functions */
#define DECL_HASH_ADD(_t)                                                          \
    static inline hash_t hash_add_##_t(hash_t hash, _t x) {                        \
        return (hash ^ (((hash_t)(x)) + 0x9E3779B9u + (hash << 6) + (hash >> 2))); \
    }

DECL_HASH_ADD(char)
DECL_HASH_ADD(int)

static inline hash_t hash_add_bytes(hash_t hash, const void *ptr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        hash = hash ^ (hash_t)((uint8_t*)ptr)[i] + 0x9E3779B9u + (hash << 6) + (hash >> 2);
    }
    return hash;
}


#endif /* HASH_H */
