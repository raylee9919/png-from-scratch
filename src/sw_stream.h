#ifndef SW_STREAM_H
#define SW_STREAM_H

#include <assert.h>
#include "sw_shared.h"


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

stream make_read_stream(buffer contents);

stream_chunk* append_chunk(stream* dst, void* src, u32 size);

u32 peek_bits(stream* buf, u32 cnt);

void discard_bits(stream* buf, u32 cnt);

u32 consume_bits(stream* buf, u32 cnt);

void flush_byte(stream* stream);

#endif
