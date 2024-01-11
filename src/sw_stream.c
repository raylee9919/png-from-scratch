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
