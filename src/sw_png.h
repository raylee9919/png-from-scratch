#ifndef SW_PNG_H
#define SW_PNG_H

#include <stdio.h>

#include "sw_image.h"
#include "sw_stream.h"

extern const u8 png_signature[8];

typedef struct {
    u8      _signature[8];
} png_header;

typedef struct {
    u32     _length;
    char    _type[4];
} png_chunk_header;

typedef struct {
    u32     _CRC;
} png_chunk_footer;

typedef struct {
    u32     _width;
    u32     _height;
    u8      _bitdepth;
    u8      _colortype;
    u8      _compressionmethod;
    u8      _filtermethod;
    u8      _interlacemethod;
} png_ihdr;

typedef struct {
    u8      _CMP;
    u8      _FLG;
} png_idat_header;

typedef struct {
    u32     _checkvalue;
} png_idat_footer;

image_u32 parse_png(stream file);

#endif
