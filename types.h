#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

/* length of comptime array */
#define ARRLEN(_arr) sizeof(_arr) / sizeof(_arr[0])
/* index of ptr in array */
#define ARR_PTR_INDEX(_arr, _p) ((int) ((_p) - &(_arr)[0]))

/* NOTE(siegmone): static qualifier naming convention */
/* internal -> private function
 * global   -> global variable
 * local    -> locally persistent var */
#define internal     static
#define global       static
#define local        static
#define force_inline static inline __attribute__((always_inline))

/* NOTE(siegmone): numeric types */
typedef uint8_t      u8;
typedef uint16_t     u16;
typedef uint32_t     u32;
typedef uint64_t     u64;
typedef int8_t       i8;
typedef int16_t      i16;
typedef int32_t      i32;
typedef int64_t      i64;
typedef float        f32;
typedef double       f64;
typedef unsigned int uint;
typedef uintptr_t    usize;
typedef intptr_t     isize;

/* NOTE(siegmone): limits */
#define U8_MIN  0
#define U16_MIN 0
#define U32_MIN 0
#define U64_MIN 0
#define U8_MAX  UINT8_MAX
#define U16_MAX UINT16_MAX
#define U32_MAX UINT32_MAX
#define U64_MAX UINT64_MAX

#define USIZE_MIN UINTPTR_MIN
#define USIZE_MAX UINTPTR_MAX

#define I8_MIN  INT8_MIN
#define I16_MIN INT16_MIN
#define I32_MIN INT32_MIN
#define I64_MIN INT64_MIN
#define I8_MAX  INT8_MAX
#define I16_MAX INT16_MAX
#define I32_MAX INT32_MAX
#define I64_MAX INT64_MAX

#define ISIZE_MIN INTPTR_MIN
#define ISIZE_MAX INTPTR_MAX

#endif /* TYPES_H */
