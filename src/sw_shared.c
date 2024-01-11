#include "sw_shared.h"

u8* advance(buffer* buf, u32 count) {
    u8* result = 0;
    
    if(buf->_count >= count) {
        result = buf->_data;
        buf->_data += count;
        buf->_count -= count;
    } else {
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
