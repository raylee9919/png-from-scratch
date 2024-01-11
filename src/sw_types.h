#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

typedef int8_t      int8;
typedef int16_t     int16;
typedef int32_t     int32;
typedef int64_t     int64;
typedef int32       bool32;
typedef bool32      b32;

typedef uint8_t     uint8;
typedef uint16_t    uint16;
typedef uint32_t    uint32;
typedef uint64_t    uint64;

typedef uint8       u8;
typedef uint16      u16;
typedef uint32      u32;
typedef uint64      u64;

typedef struct buffer
{
    u32     _count;
    u8*     _data;
} buffer;

#endif