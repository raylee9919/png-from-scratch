#ifndef SW_IMAGE_H
#define SW_IMAGE_H

#include "sw_types.h"

typedef struct image_u32
{
    u32     _width;
    u32     _height;
    u32*    _pixels;
} image_u32;

#endif
