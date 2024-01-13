#ifndef SW_STREAM_H
#define SW_STREAM_H

#include <assert.h>
#include <stdlib.h>
#include "sw_types.h"

#define LEN(array) (sizeof(array) / sizeof((array)[0]))

typedef struct stream_chunk {
    buffer _contents;
    struct stream_chunk* _next;
} stream_chunk;

typedef struct {
    buffer _contents;

    u32 _bitcount;
    u32 _bitbuf;

    b32 _underflowed;

    stream_chunk* _first;
    stream_chunk* _last;
} stream;

void refill_if_necessary(stream* file);

#define consume(file, type) (type*)consume_size(file, sizeof(type))
void* consume_size(stream* file, u32 size);

u8* advance(buffer* buf, u32 count);

void swap_endian_16(u16* value);

void swap_endian_32(u32* value);

stream make_read_stream(buffer contents);

stream_chunk* append_chunk(stream* dst, void* src, u32 size);

u32 PeekBits(stream* buf, u32 cnt);

void DiscardBits(stream* buf, u32 cnt);

u32 ConsumeBits(stream* buf, u32 cnt);

void FlushByte(stream* stream);

u32 CountBits(u32 n);

u32 Reverse(u32 bits, u32 bitCnt);

#endif
