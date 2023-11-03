#pragma once

#include <stdint.h>
#include <stddef.h>
#include <onix/arch.h>

#define CONCAT(x, y) x##y
#define RESERVED_TOKEN(x, y) CONCAT(x, y)
#define RESERVED RESERVED_TOKEN(reserved, __LINE__)

#ifndef __cplusplus
#define bool _Bool
#define true 1
#define false 0
#endif

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define _omit_frame __attribute__((optimize("omit-frame-pointer")))
#define _noreturn __attribute__((noreturn))
#define _extern extern "C"
#define _packed __attribute__((packed))
#define _weak __attribute__((__weak__))
#define _inline __attribute__((always_inline)) inline
#define _aligned(bytes) __attribute__((aligned(bytes)))
