#ifndef SW_SHARED_H
#define SW_SHARED_H

#include <malloc.h>
#include "sw_types.h"

u8* advance(buffer* buf, u32 count);

void swap_endian_16(u16* value);

void swap_endian_32(u32* value);

#endif
