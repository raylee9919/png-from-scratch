#ifndef SW_PNG_H
#define SW_PNG_H

#include <stdio.h>
#include <limits.h>

#include "sw_image.h"
#include "sw_stream.h"
#include "sw_ansi_color.h"

#define MAX_CODE_LENGTH     16
#define LL_CODE_LENGTH      15
#define D_CODE_LENGTH       15
#define CL_CODE_LENGTH      7

enum BTYPE
{
    NON_COMPRESSED, FIXED_HUFFMAN, DYNAMIC_HUFFMAN, RESERVED
};

typedef struct
{
    u8      _signature[8];
} png_header;

typedef struct
{
    u32     _length;
    char    _type[4];
} png_chunk_header;

typedef struct
{
    u32     _CRC;
} png_chunk_footer;

typedef struct
{
    u32     _width;
    u32     _height;
    u8      _bitdepth;
    u8      _colortype;
    u8      _compressionmethod;
    u8      _filtermethod;
    u8      _interlacemethod;
} png_ihdr;

typedef struct
{
    u8      _CMP;
    u8      _FLG;
} png_idat_header;

typedef struct
{
    u32     _checkvalue;
} png_idat_footer;

typedef struct
{
    u16     _symbol;
    u16     _code;
} Entry;

typedef struct
{
    u32 _max_code_length;
    u32 _entry_count;
    Entry* _entries;
} Table;

image_u32 ParsePNG(stream file);

void CLToCode(Table* table);

u32* BuildDecodeTable(Table* table);

#endif
