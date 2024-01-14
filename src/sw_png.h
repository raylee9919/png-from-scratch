#ifndef SW_PNG_H
#define SW_PNG_H

#include <stdio.h>
#include <limits.h>

#include "sw_stream.h"

#define MAX_CODE_LENGTH     16
#define LL_CODE_LENGTH      15
#define D_CODE_LENGTH       15
#define CL_CODE_LENGTH      7

enum BTYPE
{
    NON_COMPRESSED, FIXED_HUFFMAN, DYNAMIC_HUFFMAN, RESERVED
};

enum FILTER
{
    FILTER_NONE, FILTER_SUB, FILTER_UP, FILTER_AVERAGE, FILTER_PAETH
};

enum SAMPLE
{
    SAMPLE_RED, SAMPLE_BLUE, SAMPLE_GREEN, SAMPLE_ALPHA
};


typedef struct image_u32
{
    u32     _width;
    u32     _height;
    u32*    _pixels;
} image_u32;

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
} HuffmanEntry;

typedef struct
{
    u32 _maxCodeLen;
    u32 _entryCnt;
    HuffmanEntry* _entries;
} HuffmanTable;

typedef struct
{
    u16 _year;
    u8  _month;
    u8  _day;
    u8  _hour;
    u8  _minute;
    u8  _second;
} tIME_Chunk;


image_u32 ParsePNG(stream file);

HuffmanTable AllocHuffman(u32 maxCodeLen);

void ComputeHuffman(u32 symbolCnt, u32* symbolCodeLen, HuffmanTable* result);

u32 HuffmanDecode(HuffmanTable* Huffman, stream* Input);

void* AllocPixels(u32 width, u32 height, u32 bpp, u32 extraBytes);

void BuildResultPixels(u8* dst, u8* src, u32 width, u32 height, u32 colorType, u32 bppDst, u32 bppSrc);

void Unfilter(u8* pixels, u32 width, u32 height, u32 bpp);

u8 PaethPredictor(u8 left, u8 up, u8 diagonal);

#endif
