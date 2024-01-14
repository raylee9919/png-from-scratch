#include "sw_png.h"
#include "sw_stream.h"


const HuffmanEntry lenExtra[] =
{
    {3, 0},   // 257
    {4, 0},   // 258
    {5, 0},   // 259
    {6, 0},   // 260
    {7, 0},   // 261
    {8, 0},   // 262
    {9, 0},   // 263
    {10, 0},  // 264
    {11, 1},  // 265
    {13, 1},  // 266
    {15, 1},  // 267
    {17, 1},  // 268
    {19, 2},  // 269
    {23, 2},  // 270
    {27, 2},  // 271
    {31, 2},  // 272
    {35, 3},  // 273
    {43, 3},  // 274
    {51, 3},  // 275
    {59, 3},  // 276
    {67, 4},  // 277
    {83, 4},  // 278
    {99, 4},  // 279
    {115, 4}, // 280
    {131, 5}, // 281
    {163, 5}, // 282
    {195, 5}, // 283
    {227, 5}, // 284
    {258, 0}, // 285
};

HuffmanEntry distExtra[] =
{
    {1, 0},      // 0
    {2, 0},      // 1
    {3, 0},      // 2
    {4, 0},      // 3
    {5, 1},      // 4
    {7, 1},      // 5
    {9, 2},      // 6
    {13, 2},     // 7
    {17, 3},     // 8
    {25, 3},     // 9
    {33, 4},     // 10
    {49, 4},     // 11
    {65, 5},     // 12
    {97, 5},     // 13
    {129, 6},    // 14
    {193, 6},    // 15
    {257, 7},    // 16
    {385, 7},    // 17
    {513, 8},    // 18
    {769, 8},    // 19
    {1025, 9},   // 20
    {1537, 9},   // 21
    {2049, 10},  // 22
    {3073, 10},  // 23
    {4097, 11},  // 24
    {6145, 11},  // 25
    {8193, 12},  // 26
    {12289, 12}, // 27
    {16385, 13}, // 28
    {24577, 13}, // 29
};

image_u32 ParsePNG (stream file)
{
    image_u32 result;
    u32 width;
    u32 height;
    u8* pixels;

    stream* at = &file;
    
    const u8 png_signature[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    png_header* file_header = consume(at, png_header);
    assert(file_header && !memcmp(file_header->_signature, png_signature, 8));

    stream compData = {0, 0, 0, false, NULL, NULL};

    u8 colorType;
    u32 bppSrc;
    u32 bppDst;

    while (at->_contents._count > 0)
    {
        png_chunk_header* chunk_header = consume(at, png_chunk_header);
        assert(chunk_header);
        swap_endian_32(&chunk_header->_length);

        void* chunk_data = consume_size(at, chunk_header->_length);

        png_chunk_footer* chunk_footer = consume(at, png_chunk_footer);
        swap_endian_32(&chunk_footer->_CRC);


        #ifdef DEBUG
        for (u8 i = 0; i < 4; i++)
            fputc(chunk_header->_type[i], stdout);
        fputc('\n', stdout);
        #endif


        if (!memcmp(chunk_header->_type, "IHDR", 4))
        {
            png_ihdr* ihdr = (png_ihdr*)chunk_data;

            swap_endian_32(&ihdr->_width);
            swap_endian_32(&ihdr->_height);

            width  = ihdr->_width;
            height = ihdr->_height;

            colorType = ihdr->_colortype;

            switch (colorType)
            {
                case (0):
                    bppSrc = 1;
                    bppDst = 1;
                    break;
                case (2):
                    bppSrc = 3;
                    bppDst = 3;
                    break;
                case (3):
                    bppSrc = 1;
                    bppDst = 3;
                    break;
                case (4):
                    bppSrc = 2;
                    bppDst = 2;
                    break;
                case (6):
                    bppSrc = 4;
                    bppDst = 4;
                    break;
                default:
                    exit(EXIT_FAILURE);
                    break;
            }

            pixels = (u8*)AllocPixels(width, height, bppDst, 0);

            #ifdef DEBUG
            fprintf(stdout, "├ Width: %u\n", ihdr->_width);
            fprintf(stdout, "├ Height: %u\n", ihdr->_height);
            fprintf(stdout, "├ BitDepth: %u\n", ihdr->_bitdepth);
            fprintf(stdout, "├ ColorType: %u\n", ihdr->_colortype);
            fprintf(stdout, "├ CompressionMethod: %u\n", ihdr->_compressionmethod);
            fprintf(stdout, "├ FilterMethod: %u\n", ihdr->_filtermethod);
            fprintf(stdout, "└ InterlaceMethod: %u\n", ihdr->_interlacemethod);
            #endif


        }
        else if (!memcmp(chunk_header->_type, "PLTE", 4)) 
        {
            assert(chunk_header->_length % 3 == 0);
        }
        else if (!memcmp(chunk_header->_type, "IDAT", 4))
        {
            append_chunk(&compData, chunk_data, chunk_header->_length);
        }
        else if (!memcmp(chunk_header->_type, "IEND", 4))
        {
        }



    }


    png_idat_header* idat_header = consume(&compData, png_idat_header);

    u8 CM       = (idat_header->_CMP & 0xF);
    u8 CINFO    = (idat_header->_CMP >> 4);
    u8 FCHECK   = (idat_header->_FLG & 0x1F);
    u8 FDICT    = (idat_header->_FLG >> 5) & 0x1;
    u8 FLEVEL   = (idat_header->_FLG >> 6);

    assert(CM == 8 && FDICT == 0);

    #ifdef DEBUG
    printf("zlibHeader\n");
    printf("├ CM: %u\n", CM);
    printf("├ CINFO: %u\n", CINFO);
    printf("├ FCHECK: %u\n", FCHECK);
    printf("├ FDICT: %u\n", FDICT);
    printf("└ FLEVEL: %u\n", FLEVEL);
    #endif

    u8* decompData = AllocPixels(width, height, bppSrc, 1);
    u32 decompIdx = 0;

    u32 BFINAL = 0;
    while (BFINAL == 0)
    {
        BFINAL      = ConsumeBits(&compData, 1);
        u32 BTYPE   = ConsumeBits(&compData, 2);

        if (BTYPE == RESERVED)
        {
        } 
        else if (BTYPE == NON_COMPRESSED)
        {
            FlushByte(&compData);
            u16 LEN     = (u16)ConsumeBits(&compData, 16);
            u16 NLEN    = (u16)ConsumeBits(&compData, 16);
            assert(~LEN == NLEN);
            // TODO: Consume LEN bytes of uncompressed data.
        } 
        else if (BTYPE == FIXED_HUFFMAN)
        {
        } 
        else if (BTYPE == DYNAMIC_HUFFMAN)
        {
            u32 HLIT  = ConsumeBits(&compData, 5) + 257;
            u32 HDIST = ConsumeBits(&compData, 5) + 1;
            u32 HCLEN = ConsumeBits(&compData, 4) + 4;

            const u8 clSymbols[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
            assert(HCLEN <= LEN(clSymbols));

            u32 clArray[LEN(clSymbols)];
            memset(clArray, 0, sizeof(clArray));

            for (int i = 0; i < HCLEN; i++)
            {
                clArray[clSymbols[i]] = ConsumeBits(&compData, 3);
            }

            HuffmanTable clTable = AllocHuffman(CL_CODE_LENGTH);
            ComputeHuffman(LEN(clSymbols), clArray, &clTable);

            u32 lldCnt = HLIT + HDIST;
            u32 lldTable[lldCnt];

            u32 idx = 0;
            u32 repVal;
            u32 repCnt;
            while(idx < lldCnt)
            {
                u32 clSymbol = HuffmanDecode(&clTable, &compData);
                assert(clSymbol <= 18);
                if(clSymbol <= 15)
                {
                    repVal = clSymbol;
                    repCnt = 1;
                }
                else if(clSymbol == 16)
                {
                    assert(idx > 0);
                    repVal = lldTable[idx - 1];
                    repCnt = 3 + ConsumeBits(&compData, 2);
                }
                else if(clSymbol == 17)
                {
                    repVal = 0;
                    repCnt = 3 + ConsumeBits(&compData, 3);
                }
                else if(clSymbol == 18)
                {
                    repVal = 0;
                    repCnt = 11 + ConsumeBits(&compData, 7);
                }

                while(repCnt--)
                {
                    lldTable[idx++] = repVal;
                }
            }

            assert(idx == lldCnt);
            free(clTable._entries);

            HuffmanTable llTable = AllocHuffman(LL_CODE_LENGTH);
            HuffmanTable dTable  = AllocHuffman(D_CODE_LENGTH);
            ComputeHuffman(HLIT,  lldTable,         &llTable);
            ComputeHuffman(HDIST, lldTable + HLIT,  &dTable);

            while (1)
            {
                u32 llSymbol = HuffmanDecode(&llTable, &compData);
                assert(llSymbol <= 285);

                if (llSymbol <= 255)
                {
                    llSymbol &= 0xFF;
                    decompData[decompIdx++] = (u8)llSymbol;
                }
                else if (llSymbol == 256)
                {
                    #ifdef DEBUG
                    printf("End of sample.\n");
                    #endif
                    break;
                }
                else
                {
                    u32 lenExtraCnt = lenExtra[llSymbol - 257]._code;
                    u32 len = lenExtra[llSymbol - 257]._symbol + ConsumeBits(&compData, lenExtraCnt);

                    u32 dSymbol = HuffmanDecode(&dTable, &compData);
                    u32 distExtraCnt = distExtra[dSymbol]._code;
                    u32 dist = distExtra[dSymbol]._symbol + ConsumeBits(&compData, distExtraCnt);

                    u32 back = (decompIdx - dist);

                    while (len--)
                    {
                        decompData[decompIdx++] = decompData[back++];
                    }
                }
            }

        }



    }

    #ifdef DEBUG
    printf("%u / %u\n", decompIdx, bppSrc * width * height + height);
    printf("BPP_decompPixels: %u\n", bppSrc);
    printf("BPP_finalPixels: %u\n", bppDst);
    #endif
    assert(decompIdx == bppSrc * width * height + height);

    BuildResultPixels(pixels, decompData, width, height, colorType, bppDst, bppSrc);
    result._width  = width;
    result._height = height;
    result._pixels = (u32*)pixels;

    return(result);
}

HuffmanTable AllocHuffman(u32 maxCodeLen)
{
    assert(maxCodeLen <= MAX_CODE_LENGTH);
    
    HuffmanTable result;

    result._maxCodeLen = maxCodeLen;
    result._entryCnt = (1 << maxCodeLen);
    // TODO: free this
    // CL, LL, D
    result._entries = (HuffmanEntry*)malloc(result._entryCnt * sizeof(HuffmanEntry));

    return(result);
}

void ComputeHuffman(u32 symbolCnt, u32* symbolCodeLen, HuffmanTable* result)
{
    u32 CodeLengthHist[MAX_CODE_LENGTH] = {};
    for(u32 SymbolIndex = 0;
            SymbolIndex < symbolCnt;
            ++SymbolIndex)
    {
        u32 Count = symbolCodeLen[SymbolIndex];
        assert(Count <= LEN(CodeLengthHist));
        ++CodeLengthHist[Count];
    }

    u32 NextUnusedCode[MAX_CODE_LENGTH];
    NextUnusedCode[0] = 0;
    CodeLengthHist[0] = 0;
    for(u32 BitIndex = 1;
            BitIndex < LEN(NextUnusedCode);
            ++BitIndex)
    {
        NextUnusedCode[BitIndex] = ((NextUnusedCode[BitIndex - 1] +
                    CodeLengthHist[BitIndex - 1]) << 1);
    }

    for(u32 SymbolIndex = 0;
            SymbolIndex < symbolCnt;
            ++SymbolIndex)
    {
        u32 CodeLengthInBits = symbolCodeLen[SymbolIndex];
        if(CodeLengthInBits)
        {
            assert(CodeLengthInBits < LEN(NextUnusedCode));
            u32 Code = NextUnusedCode[CodeLengthInBits]++;

            u32 ArbitraryBits = result->_maxCodeLen - CodeLengthInBits;
            u32 EntryCount = (1 << ArbitraryBits);

            for(u32 EntryIndex = 0;
                    EntryIndex < EntryCount;
                    ++EntryIndex)
            {
                u32 BaseIndex = (Code << ArbitraryBits) | EntryIndex;
                u32 Index = Reverse(BaseIndex, result->_maxCodeLen);

                HuffmanEntry *Entry = result->_entries + Index;

                u32 Symbol = (SymbolIndex);
                Entry->_code = (u16)CodeLengthInBits;
                Entry->_symbol = (u16)Symbol;

                assert(Entry->_code == CodeLengthInBits);
                assert(Entry->_symbol == Symbol);
            }
        }
    }
}

u32 HuffmanDecode(HuffmanTable* Huffman, stream* Input)
{
    u32 EntryIndex = PeekBits(Input, Huffman->_maxCodeLen);
    assert(EntryIndex < Huffman->_entryCnt);

    HuffmanEntry Entry = Huffman->_entries[EntryIndex];

    u32 result = Entry._symbol;
    DiscardBits(Input, Entry._code);
    assert(Entry._code);

    return(result);
}

void* AllocPixels(u32 width, u32 height, u32 bpp, u32 extraBytes)
{
    void* result = malloc(width * height * bpp + (extraBytes * height));

    return(result);
}

void BuildResultPixels(u8* dst, u8* src, u32 width, u32 height, u32 colorType, u32 bppDst, u32 bppSrc)
{
    Unfilter(src, width, height, bppSrc);

    if (colorType == 3) // NOTE: if uses PLTE
    {
        //TOOD: src->PLTC->dst
    }
    else
    {
        u32 widthBytes = width * bppDst;
        for (u32 row = 0; row < height; row++)
        {
            u32 baseIdxDst = (row * width * bppDst);
            u32 baseIdxSrc = (row * width * bppSrc) + 1;
            for (u32 offset; offset < widthBytes; offset++)
            {
                dst[baseIdxDst + offset] = src[baseIdxSrc + offset];
            }
        }
    }

    free(src);
}

void Unfilter(u8* pixels, u32 width, u32 height, u32 bpp)
{
    u32 widthByte  = width  * bpp;
    u32 heightByte = height * bpp;

    for (int row = 0; row < height; row++)
    {
        u32 baseIdx = row * (widthByte + 1) + 1;
        u8 filter = pixels[baseIdx - 1];

        switch(filter)
        {
            case(FILTER_NONE):
            {
            } break;
            case(FILTER_SUB):
            {
                u32 stride = bpp;
                for(;stride < widthByte;
                    stride += bpp)
                {
                    for (u32 offset = 0; offset < bpp; offset++)
                    {
                        u32 idx = (baseIdx + stride + offset);
                        pixels[idx] = pixels[idx] + pixels[idx - bpp];
                    }
                }
            } break;
            case(FILTER_UP):
            {
                if (row != 0)
                {
                    u32 stride = 0;
                    for(;stride < widthByte;
                        stride += bpp)
                    {
                        for (u32 offset = 0; offset < bpp; offset++)
                        {
                            u32 idx = (baseIdx + stride + offset);
                            pixels[idx] = pixels[idx] + pixels[idx - widthByte - 1];
                        }
                    }
                }
            } break;
            case(FILTER_AVERAGE):
            {
                u32 stride = 0;
                for(;stride < widthByte;
                    stride += bpp)
                {
                    for (u32 offset = 0; offset < bpp; offset++)
                    {
                        u32 idx = (baseIdx + stride + offset);
                        u8 left = (baseIdx == 0) ? 0 : pixels[idx - bpp];
                        u8 up = (row == 0) ? 0 : pixels[idx - widthByte - 1];
                        pixels[idx] = pixels[idx] + ((left + up) >> 2);
                    }
                }
            } break;
            case(FILTER_PAETH):
            {
                u32 stride = 0;
                for(;stride < widthByte;
                    stride += bpp)
                {
                    for (u32 offset = 0; offset < bpp; offset++)
                    {
                        u32 idx = (baseIdx + stride + offset);
                        u8 left = (baseIdx == 0) ? 0 : pixels[idx - bpp];
                        u8 up = (row == 0) ? 0 : pixels[idx - widthByte - 1];
                        u8 diagonal = (baseIdx == 0 || row == 0) ?
                            0 : pixels[idx - widthByte - 2];
                        pixels[idx] = pixels[idx] + PaethPredictor(left, up, diagonal);
                    }
                }
            } break;
            default:
            {
                fprintf(stderr, "ERROR: Invalid filter %u at row %u.\n", filter, row + 1);
                exit(EXIT_FAILURE);
            } break;
        }
    }

}

u8 PaethPredictor(u8 a, u8 b, u8 c)
{
    u32 p = a + b - c;
    u32 pa = Diff(p, a);
    u32 pb = Diff(p, b);
    u32 pc = Diff(p, c);

    return(Min(Min(pa, pb), pc));
}

u32 Diff(u32 x, u32 y)
{
    return((x > y) ? x - y : y - x);
}

u32 Min(u32 x, u32 y)
{
    return((x < y) ? x : y);
}
