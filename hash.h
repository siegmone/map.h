#ifndef HASH_H
#define HASH_H

#include <stddef.h>
#include <stdint.h>

#include "types.h"

typedef uint64_t hash_t;

/* hash functions */
/* use it as a template to add a hash function for the type
 * */
#define DECL_HASH_ADD(_t)                                                          \
    force_inline hash_t hash_add_##_t(hash_t hash, _t x) {                         \
        return (hash ^ (((hash_t)(x)) + 0x9E3779B9u + (hash << 6) + (hash >> 2))); \
    }

DECL_HASH_ADD(char)
DECL_HASH_ADD(int)

DECL_HASH_ADD(u8);
DECL_HASH_ADD(u16);
DECL_HASH_ADD(u32);
DECL_HASH_ADD(u64);

DECL_HASH_ADD(i8);
DECL_HASH_ADD(i16);
DECL_HASH_ADD(i32);
DECL_HASH_ADD(i64);

DECL_HASH_ADD(usize);
DECL_HASH_ADD(isize);

force_inline hash_t hash_add_str(hash_t hash, const char *s) {
    while (*s) {
        hash = hash_add_char(hash, *s);
        s++;
    }
    return hash;
}

force_inline hash_t hash_add_f32(hash_t hash, f32 x) {
    return hash_add_u32(hash, *(u32 *)&x);
}

force_inline hash_t hash_add_f64(hash_t hash, f64 x) {
    return hash_add_u64(hash, *(u64 *)&x);
}

force_inline hash_t hash_add_uintptr(hash_t hash, uintptr_t x) {
    return (hash ^ (((hash_t)(x)) + 0x9E3779B9u + (hash << 6) + (hash >> 2)));
}

force_inline hash_t hash_add_ptr(hash_t hash, void *ptr) {
    return hash_add_uintptr(hash, (uintptr_t)ptr);
}

force_inline hash_t hash_add_ptrs(hash_t hash, void **ptrs, int n) {
    for (int i = 0; i < n; i++) {
        hash = hash_add_ptr(hash, ptrs[i]);
    }
    return hash;
}

force_inline hash_t hash_add_bytes(hash_t hash, const void *ptr, size_t size) {
    for (size_t i = 0; i < size; i++) {
        hash = hash_add_u8(hash, ((u8 *)ptr)[i]);
    }
    return hash;
}

#endif /* HASH_H */
