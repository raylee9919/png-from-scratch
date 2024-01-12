#include "sw_stream.h"

void refill_if_necessary(stream* file) {
    if ((file->_contents._count == 0) && file->_first) {
        stream_chunk* this = file->_first;
        file->_contents = this->_contents;
        file->_first = this->_next;
        free(this);
    }
}

void* consume_size(stream* file, u32 size) {
    refill_if_necessary(file);
    
    void* result = advance(&file->_contents, size);
    if(!result) {
        file->_underflowed = true;
    }
    
    assert(!file->_underflowed);
    
    return(result);
}

stream make_read_stream(buffer contents) {
    stream result = {};
    
    result._contents = contents;
    
    return(result);
}

stream_chunk* append_chunk(stream* dst, void* src, u32 size) {
    stream_chunk* chunk = (stream_chunk*)malloc(sizeof(stream_chunk));
    chunk->_contents._count = size;
    chunk->_contents._data = (u8*)src;
    chunk->_next = NULL;
    
    if (dst->_last == NULL) {
        dst->_first = chunk;
        dst->_last = chunk;
    } else {
        dst->_last->_next = chunk;
        dst->_last = chunk;
    }
    
    return(chunk);
}

u32 peek_bits(stream* buf, u32 cnt) {
    assert(cnt <= 32);
    
    u32 result = 0;
    
    while((buf->_bitcount < cnt) && !buf->_underflowed) {
        u32 byte = *consume(buf, u8);
        buf->_bitbuf |= (byte << buf->_bitcount);
        buf->_bitcount += 8;
    }
    
    result = buf->_bitbuf & ((1 << cnt) - 1);
    
    return(result);
}

void discard_bits(stream* buf, u32 cnt) {
    buf->_bitcount -= cnt;
    buf->_bitbuf >>= cnt;
}

u32 consume_bits(stream* buf, u32 cnt) {
    u32 result = peek_bits(buf, cnt);
    discard_bits(buf, cnt);
    
    return(result);
}

void flush_byte(stream* stream) {
    u32 cnt = (stream->_bitcount % 8);
    consume_bits(stream, cnt);
}
