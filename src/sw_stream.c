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

u8* advance(buffer* buf, u32 count)
{
    u8* result = 0;
    
    if(buf->_count >= count)
    {
        result = buf->_data;
        buf->_data += count;
        buf->_count -= count;
    } 
    else
    {
        buf->_data += buf->_count;
        buf->_count = 0;
    }
    
    return(result);
}

void swap_endian_16 (u16* value) {
    u16 v = (*value);
    *value = ((v << 8) | (v >> 8));
}

void swap_endian_32 (u32* value) {
    u32 v = (*value);
    *value = ((v << 24) |
              ((v & 0xFF00) << 8) |
              ((v >> 8) & 0xFF00) |
              (v >> 24));
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
    
    if (dst->_last == NULL)
    {
        dst->_first = chunk;
        dst->_last = chunk;
    } 
    else
    {
        dst->_last->_next = chunk;
        dst->_last = chunk;
    }
    
    return(chunk);
}

u32 PeekBits(stream* buf, u32 cnt) {
    assert(cnt <= 32);
    
    u32 result = 0;
    
    while((buf->_bitcount < cnt) &&
            !buf->_underflowed)
    {
        u32 byte = *consume(buf, u8);
        buf->_bitbuf |= (byte << buf->_bitcount);
        buf->_bitcount += 8;
    }
    
    result = buf->_bitbuf & ((1 << cnt) - 1);
    
    return(result);
}

void DiscardBits(stream* buf, u32 cnt) {
    buf->_bitcount -= cnt;
    buf->_bitbuf >>= cnt;
}

u32 ConsumeBits(stream* buf, u32 cnt) {
    u32 result = PeekBits(buf, cnt);
    DiscardBits(buf, cnt);
    
    return(result);
}

void FlushByte(stream* stream) {
    u32 cnt = (stream->_bitcount % 8);
    ConsumeBits(stream, cnt);
}

u32 CountBits(u32 n) 
{ 
   u32 result = 0; 
   while (n) 
   { 
        result++; 
        n >>= 1; 
    } 
    return(result); 
}

u32 Reverse(u32 bits, u32 bitCnt)
{
    u32 result = 0;
    
    for(u32 i = 0; i <= (bitCnt / 2); ++i)
    {
        u32 inv = (bitCnt - (i + 1));
        result |= ((bits >> i) & 0x1) << inv;
        result |= ((bits >> inv) & 0x1) << i;
    }
    
    return(result);
}
