#include "sw_png.h"
#include "sw_stream.h"

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

    while (at->_contents._count > 0)
    {
        png_chunk_header* chunk_header = consume(at, png_chunk_header);
        assert(chunk_header);
        swap_endian_32(&chunk_header->_length);

        void* chunk_data = consume_size(at, chunk_header->_length);

        png_chunk_footer* chunk_footer = consume(at, png_chunk_footer);
        swap_endian_32(&chunk_footer->_CRC);


        for (u8 i = 0; i < 4; i++)
            fputc(chunk_header->_type[i], stdout);
        fputc('\n', stdout);

        if (!memcmp(chunk_header->_type, "IHDR", 4))
        {
            png_ihdr* ihdr = (png_ihdr*)chunk_data;

            swap_endian_32(&ihdr->_width);
            swap_endian_32(&ihdr->_height);

            width  = ihdr->_width;
            height = ihdr->_height;
            pixels = (u8*)AllocPixels(width, height, 4);

            fprintf(stdout, "├ Width: %u\n", ihdr->_width);
            fprintf(stdout, "├ Height: %u\n", ihdr->_height);
            fprintf(stdout, "├ BitDepth: %u\n", ihdr->_bitdepth);
            fprintf(stdout, "├ ColorType: %u\n", ihdr->_colortype);
            fprintf(stdout, "├ CompressionMethod: %u\n", ihdr->_compressionmethod);
            fprintf(stdout, "├ FilterMethod: %u\n", ihdr->_filtermethod);
            fprintf(stdout, "└ InterlaceMethod: %u\n", ihdr->_interlacemethod);

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

    printf("zlibHeader\n");
    png_idat_header* idat_header = consume(&compData, png_idat_header);

    u8 CM       = (idat_header->_CMP & 0xF);
    u8 CINFO    = (idat_header->_CMP >> 4);
    u8 FCHECK   = (idat_header->_FLG & 0x1F);
    u8 FDICT    = (idat_header->_FLG >> 5) & 0x1;
    u8 FLEVEL   = (idat_header->_FLG >> 6);

    assert(CM == 8 && FDICT == 0);

    printf("├ CM: %u\n", CM);
    printf("├ CINFO: %u\n", CINFO);
    printf("├ FCHECK: %u\n", FCHECK);
    printf("├ FDICT: %u\n", FDICT);
    printf("└ FLEVEL: %u\n", FLEVEL);

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
            assert(lldCnt <= LEN(lldTable));

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




        }



    }

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

void* AllocPixels(u32 width, u32 height, u32 bpp)
{
    void* result = malloc(width * height * bpp);
    
    return(result);
}
